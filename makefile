commondir = /Users/angelinajo/Documents/Liuplugg/Datorgrafik/common/

all : project

project : project.c $(commondir)GL_utilities.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)LoadTGA.c $(commondir)Mac/MicroGlut.m
	gcc -Wall -o project -I$(commondir) -I../common/Mac -DGL_GLEXT_PROTOTYPES project.c $(commondir)GL_utilities.c $(commondir)loadobj.c $(commondir)VectorUtils3.c $(commondir)LoadTGA.c $(commondir)Mac/MicroGlut.m -framework Cocoa -framework OpenGL -lm -Wno-deprecated


clean :
	rm project
