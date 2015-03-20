TEMPLATE 	= app

TARGET 		= webGLRtiMaker
LANGUAGE 	= C++
CONFIG 		+= qt debug_and_release console
QT          += xml gui
 

MOC_DIR 	= moc

SOURCES 	= image.cpp \
			  rti.cpp \
			  splitter.cpp \
			  qgetopt.cpp \
			  main.cpp \ 
   
HEADERS 	= multilayer.h \
			  rti.h \
			  image.h \
			  splitter.h \
			  qgetopt.h \
