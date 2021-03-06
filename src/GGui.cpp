#include "GGui.h"

GGui gui;

//#include "Error.h"
GGui::GGui()
	:is_data_changed(false)
	,is_setup(false)
	,state(STATE_NONE)
	,x(0)
	,y(0)
{
}

void GGui::enableEventListeners() {
	ofRegisterMouseEvents(this);
}
void GGui::disableEventListeners() {
	ofUnregisterMouseEvents(this);
}

//void GGui::setup(ofEventArgs& args) {
void GGui::setup() {
	glGenBuffers(1, &vbo_vertices); eglGetError();
	glGenBuffers(1, &vbo_texcoords); eglGetError();

	checkAssetFiles();
	x = 0;
	y = 0;
	metrics.label_x_offset = 10;
	metrics.label_y_offset = 17;
	metrics.label_width = 165;
	metrics.row_width = 400;
	metrics.row_height = 25;
	label_font.loadFont("gui/gui_font.ttf",8, true, true);
	value_font.loadFont("gui/gui_font.ttf",8, true, true);
	
	createGuiTexture();
	
	createOrthoProjectionMatrix();
	
	// We're not drawing the background anymore... though leave the code 
	// here as I might change this in the future.....
	// Create background quad
	// -------------------------------------------------------------------------
	float h = 1440; // tmp height
	//	addVertex(x,y);
	//	addVertex(x+metrics.row_width,y);
	//	addVertex(x+metrics.row_width,y+h);
	//	addVertex(x,y+h);

			
	//	addTexCoord(GTCX(48), GTCY(128));
	//	addTexCoord(GTCX(448), GTCY(128));
	//	addTexCoord(GTCX(448), GTCY(128+h));
	//	addTexCoord(GTCX(48), GTCY(128+h));
	
	ofAddListener(ofEvents.update, this, &GGui::update);// we need to put this outise enableEventListeners
	enableEventListeners();
	GGuis::getInstance()->addGui(this);
	is_setup = true;
	show();
}

void GGui::createOrthoProjectionMatrix() {
	float w = ofGetWidth();
	float h = ofGetHeight();
	float n = 0.1;
	float f = 10.0;
	float m[16];
	
	ortho_projection[1]  = 0.0f;
	ortho_projection[2]  = 0.0f;
	ortho_projection[3]  = 0.0f;
	ortho_projection[4]  = 0.0f;
	ortho_projection[6]  = 0.0f;
	ortho_projection[7]  = 0.0f;
	ortho_projection[8]  = 0.0f;
	ortho_projection[9]  = 0.0f;
	ortho_projection[11] = 0.0f;
	ortho_projection[15] = 1.0f;

	float fmn = f - n;
	ortho_projection[0]  = 2.0f / w;
	ortho_projection[5]  = 2.0f / -h;
	ortho_projection[10] = -2.0f / fmn;
	ortho_projection[12] = -(w)/w;
	ortho_projection[13] = -(h)/-h;
	ortho_projection[14] = -(f+n)/fmn;
}


GSlider& GGui::addFloat(string title, float& value) {
	GSlider* slider = new GSlider(this, &value);
	slider->setLabel(title);
	slider->width = metrics.row_width;
	addObject(slider);
	return *slider;
}

GCheckbox& GGui::addBool(string title, bool& flag) {
	GCheckbox* box = new GCheckbox(this, &flag);
	box->setLabel(title);
	box->width = metrics.row_width;
	box->height = metrics.row_height;

	addObject(box);
	return *box;	
}

GButton& GGui::addButton(string title, string message) {
	GButton* button = new GButton(this);
	button->width = metrics.row_width;
	button->height = metrics.row_height + 10;

	button->setLabel(title);
	button->setMessage(message);
	addObject(button);
	return *button;
}

GSeparator& GGui::addSeparator() {
	GSeparator* sep = new GSeparator(this);
	sep->width = metrics.row_width;
	sep->height = 25;
	addObject(sep);
	return *sep;
}

void GGui::update(ofEventArgs& args) {
	if(is_data_changed) {
		updateVertices();	
	}
	if(state != STATE_NONE) {
		if(state == STATE_HIDING) {
			int now = ofGetElapsedTimeMillis();
			float p = (float)(end_tween_on-now)/300;
			x = (-metrics.row_width) * (1.0 - p);
			if(p < 0) {
				state = STATE_HIDDEN;
				disableEventListeners();
				x = -metrics.row_width;
			}

			
		}
		else if(state == STATE_SHOWING) {
			int now = ofGetElapsedTimeMillis();
			float p = (float)(end_tween_on-now)/300;
			x = -metrics.row_width * p;
			if(p < 0) {
				state = STATE_VISIBLE;
				enableEventListeners();
				x = 0.0f;
			}
		}
	}
}

void GGui::updateVertices() {
	
	// vertices.
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices); eglGetError();
	glBufferData(
		GL_ARRAY_BUFFER
		,sizeof(ofVec2f) * vertices.size()
		,&vertices[0].x
		,GL_STATIC_DRAW
	); eglGetError(); 
	glBindBuffer(GL_ARRAY_BUFFER, 0);  eglGetError();
	
	
	// texcoords
	glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoords); eglGetError();
	glBufferData(
		GL_ARRAY_BUFFER
		,sizeof(ofVec2f) * texcoords.size()
		,&texcoords[0].x
		,GL_STATIC_DRAW
	); eglGetError();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	is_data_changed = false;
}

void GGui::draw() {
	// get current model and projection matrix. We draw the gui in orthographic
	// mode and we will reset the projection and modelview matrices after drawing
	// the gui.
	GLdouble curr_proj[16];
	GLdouble curr_model[16];
	GLdouble curr_texture[16];
	glGetDoublev(GL_PROJECTION_MATRIX, curr_proj);
	glGetDoublev(GL_MODELVIEW_MATRIX, curr_model);
	glGetDoublev(GL_TEXTURE_MATRIX, curr_texture);

	// we use non-normalized texcoords
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(ortho_projection);	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0,0,-5);
	
	bool cull_enabled = false;
	if(glIsEnabled(GL_CULL_FACE)) {
		glDisable(GL_CULL_FACE);
		cull_enabled = true;
	}
	
	bool depth_enabled = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);

		// draw gui.
		glPushMatrix();
			glColor4f(1.0, 1.0, 1.0, 1.0); 
			glTranslatef(x,y,0);
			
			// texture.
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, gui_texture); eglGetError();
			glEnable(GL_BLEND); eglGetError();
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); eglGetError()
			
			// texcoords
			glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoords); eglGetError();
			glEnableClientState(GL_TEXTURE_COORD_ARRAY); eglGetError();
			glTexCoordPointer(2,GL_FLOAT, 0, 0); eglGetError();

			// vertices.
			glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices); eglGetError();
			glEnableClientState(GL_VERTEX_ARRAY);  eglGetError();
			glVertexPointer(2, GL_FLOAT,0,0);  eglGetError();

			// draw background.	
			glDrawArrays(GL_QUADS, 0, vertices.size());  eglGetError();
			
			glBindBuffer(GL_ARRAY_BUFFER, 0);  eglGetError();
			
			glBindTexture(GL_TEXTURE_2D, 0); eglGetError();
			
			float mx = ofGetMouseX();
			float my = ofGetMouseY();
			const int num = objects.size();
			GObject** ptr = (num != 0) ? &objects.front() : NULL;
		//	glColor3f(0.9,0.9,0.8);
			for(int i = 0; i < num; ++i) {
				// debug draw
				//	glBegin(GL_LINE_LOOP);
				//		glVertex2f(ptr[i]->x, ptr[i]->y);
				//		glVertex2f(ptr[i]->x+ptr[i]->width, ptr[i]->y);
				//		glVertex2f(ptr[i]->x+ptr[i]->width, ptr[i]->y+ptr[i]->height);
				//		glVertex2f(ptr[i]->x, ptr[i]->y+ptr[i]->height);
				//	glEnd();
				ptr[i]->draw();
			}
			glColor3f(1,1,1);
		glPopMatrix();

	glDisable(GL_BLEND); eglGetError();
	glDisableClientState(GL_VERTEX_ARRAY);  eglGetError();
	glDisableClientState(GL_TEXTURE_COORD_ARRAY); eglGetError();
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// reset previous projection and modelview matrices
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixd(curr_texture);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(curr_proj);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(curr_model);
	
	if(cull_enabled) {
		glEnable(GL_CULL_FACE);
	}
	if(depth_enabled) {
		glEnable(GL_DEPTH_TEST);
	}
}


void GGui::createGuiTexture() {
	glEnable(GL_TEXTURE_2D); eglGetError();
	glGenTextures(1,&gui_texture); eglGetError();
	glBindTexture(GL_TEXTURE_2D, gui_texture); eglGetError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); eglGetError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); eglGetError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); eglGetError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); eglGetError();

	ofImage img;
	if(!img.loadImage("gui/gui.png")) {
		ofLog(OF_LOG_ERROR, "Cannot load gui image");
		glBindTexture(GL_TEXTURE_2D, 0);
		return;		
	}
	
	glTexImage2D(
		 GL_TEXTURE_2D, 0
		,GL_RGBA, img.getWidth(), img.getHeight(), 0
		,GL_RGBA, GL_UNSIGNED_BYTE, img.getPixels()
	);
	eglGetError();
	glBindTexture(GL_TEXTURE_2D,0);
}

void GGui::addObject(GObject* obj) {
	if(!is_setup) {
		setup();
	}
	
	objects.push_back(obj);
	
	// create a name for the object.
	if(obj->label != "") {
		string clean_name = createCleanName(obj->label);
		obj->setCleanName("main." +clean_name);
	}

	positionObjects();
	obj->setup();
}

void GGui::positionObjects() {
	float xx = x;
	float yy = y+15;
	const int num = objects.size();
	GObject** ptr = (num != 0) ? &objects.front() : NULL;
	for(int i = 0; i < num; ++i) {
		ptr[i]->x = xx;
		ptr[i]->y = yy;
		yy += ptr[i]->getHeight(); 
	}
}

void GGui::mouseMoved(ofMouseEventArgs& args) {
	const int num = objects.size();
	GObject** ptr = (num != 0) ? &objects.front() : NULL;
	GObject* obj = NULL;
	for(int i = 0; i < num; ++i) {
		obj = ptr[i];
		obj->is_mouse_inside = GINSIDE_OBJECT(ptr[i], args.x, args.y);
		if(obj->is_mouse_inside && obj->state == GObject::GSTATE_NONE) {
			obj->state = GObject::GSTATE_ENTER;
			obj->mouseEnter(args.x, args.y);
		}
		else if(!obj->is_mouse_inside && obj->state == GObject::GSTATE_ENTER) {
			obj->mouseLeave(args.x, args.y);
			obj->state = GObject::GSTATE_NONE;
		}
		ptr[i]->mouseMoved(args.x, args.y);
	}
}

void GGui::mouseDragged(ofMouseEventArgs& args) {
	const int num = objects.size();
	GObject** ptr = (num != 0) ? &objects.front() : NULL;
	for(int i = 0; i < num; ++i) {
		ptr[i]->mouseDragged(args.x, args.y);
	}
}

void GGui::mousePressed(ofMouseEventArgs& args) {
	const int num = objects.size();
	GObject** ptr = (num != 0) ? &objects.front() : NULL;
	for(int i = 0; i < num; ++i) {
		ptr[i]->is_mouse_down_inside = (GINSIDE_OBJECT(ptr[i], args.x, args.y));
		ptr[i]->mousePressed(args.x, args.y);
	}
}

void GGui::mouseReleased(ofMouseEventArgs& args) {
	const int num = objects.size();
	GObject** ptr = (num != 0) ? &objects.front() : NULL;
	for(int i = 0; i < num; ++i) {
		ptr[i]->is_mouse_inside = (GINSIDE_OBJECT(ptr[i], args.x, args.y));
		if(ptr[i]->is_mouse_down_inside && ptr[i]->is_mouse_inside) {
			ptr[i]->mouseClick(args.x, args.y);
		}
		ptr[i]->is_mouse_down_inside = false;
		ptr[i]->mouseReleased(args.x, args.y);
	}
}


// kind of a hack to fix that the user does not have to copy the 
// image to their executable dir themself.
void GGui::checkAssetFiles() {
	string dest_path = ofToDataPath("gui/gui.png",true);
	ofFile dest_file(dest_path);
	if(!dest_file.exists()) {
		ofFile file(__FILE__);
		ofDirectory gui_src_dir(file.getEnclosingDirectory() +"assets/");
		gui_src_dir.copyTo("gui",true,false);
	}
}

void GGui::save(string filename) {
	int num = objects.size();
	GINI ini;
	GObject** ptr = (num == 0) ? NULL : &objects.front();
	for(int i = 0; i < num; ++i) {
		ptr[i]->save(&ini);
	}
	ini.save(ofToDataPath("gui/" +filename, true));
}

void GGui::load(string filename) {
	int num = objects.size();
	GINI ini;
	ini.load(ofToDataPath("gui/" +filename,true));

	GObject** ptr = (num == 0) ? NULL : &objects.front();
	for(int i = 0; i < num; ++i) {
		ptr[i]->load(&ini);
	}
}