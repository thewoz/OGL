INCLUDE_INSTALL_PATH=/usr/local/include
LIBRARY_INSTALL_PATH=/usr/local/lib

LIBRARY_NAME=imgui

all: install

compile:
	@mkdir -p ./build/lib
	@mkdir -p ./build/include
	g++ -march=native --std=c++11 -Os -c -DIMGUI_IMPL_OPENGL_LOADER_GLAD ./code/imgui_draw.cpp -o ./build/imgui_draw.o
	g++ -march=native --std=c++11 -Os -c -DIMGUI_IMPL_OPENGL_LOADER_GLAD ./code/imgui_widgets.cpp -o ./build/imgui_widgets.o
	g++ -march=native --std=c++11 -Os -c -DIMGUI_IMPL_OPENGL_LOADER_GLAD ./code/imgui_tables.cpp -o ./build/imgui_tables.o
	g++ -march=native --std=c++11 -Os -c -DIMGUI_IMPL_OPENGL_LOADER_GLAD ./code/imgui_impl_opengl3.cpp -o ./build/imgui_impl_opengl3.o
	g++ -march=native --std=c++11 -Os -c -DIMGUI_IMPL_OPENGL_LOADER_GLAD ./code/imgui_impl_glfw.cpp -o ./build/imgui_impl_glfw.o
	g++ -march=native --std=c++11 -Os -c -DIMGUI_IMPL_OPENGL_LOADER_GLAD ./code/imgui.cpp -o ./build/imgui.o
	g++ -march=native --std=c++11 -Os -c ./code/imgui_neo_internal.cpp -o ./build/imgui_neo_internal.o
	g++ -march=native --std=c++11 -Os -c ./code/imgui_neo_sequencer.cpp -o ./build/imgui_neo_sequencer.o
	@ar rcs ./build/lib/libimgui.a ./build/*.o
	@cp ./code/*.h ./build/include
	@cp imgui.hpp ./build/include

clear:
	@rm -rf ./build
	
install: compile
	@mkdir -p $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)/
	@mkdir -p $(LIBRARY_INSTALL_PATH)
	@cp ./build/include/*.h* $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)/
	@cp ./build/lib/libimgui.a $(LIBRARY_INSTALL_PATH)

uninstall:
	@rm -rf $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)/
	@rm -rf $(LIBRARY_INSTALL_PATH)/$(LIBRARY_NAME)/






