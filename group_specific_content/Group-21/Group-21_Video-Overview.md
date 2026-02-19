**Group 21 Video Overview Link & Script**  
Link to video: https://mediaspace.msu.edu/media/Group-21_Video-Overview/1_rynr1n00

**Andreea (ImageManager):**   
This class holds an object that handles image assets by name, and holds them until they are not needed anymore. An unordered map of strings holds the images. The way to load in an image is to pass the imageID which can be anything memorable to describe the image such as stop sign or plant1, and the second parameter would be the path where the image is being held. Which would typically be in the images directory. If the load function is called without one of those parameters it will throw an error. If you’d like to get the imagepath to use in another class without having to look through the image directory, you can call GetImage and it will return the path if one exists with that imageID. If you’d like to look up if an image exists you can use HasImage with the parameter of imageID, and it will return true or false depending on if it exists. To erase an image, you can call Remove with the imageID, and it will be deleted from the map. To clear all images, you can use the Clear function and the map will be empty.  

**Divya (Text):**   
The Text class is a way to make it simpler for other programmers to implement text in their classes that they want displayed. It modifies the text based on features such as the font family, the text color, the style of the text, whether it is bold/underline/italics, the size of the text, as well as whether or not the text is capitalized. As you can see, in this code these values are stored as the getters and setters. The class uses the Qt Graphics library to display the text (show the Qt section), and uses specific functions such as setBold, setCapitalization, setFont, setColor, as you can see in the implementation file.  
   
This testing file I made demonstrates how someone else would use the class. Let’s say someone is doing a Flower related class and wants to display the name of a flower. The user would make sure to include the Qt libraries as well as the Text file in the header. Once the user is finished compiling the program, the text with the flower name will show up, as it did here

**Annika (Menu):**  
This is the Menu class for group 21\. The purpose of this class is to act as the logic layer behind a graphical menu in a user interface. It stores menu options, tracks state like which option is selected, and responds to navigation input so the UI can stay consistent and predictable.

The menu is made up of Item objects. Each item has a unique id, a programmer-facing key used for reliable lookups, and a user-facing label that is shown on screen. Items also have state flags: enabled controls whether an option can be interacted with, visible controls whether it should appear at all, and selected indicates which item is currently highlighted.

To support dynamic interface behavior, items can also have predicates: enabledIf and visibleIf. These are functions that return true or false, allowing an item’s enabled or visible state to depend on the current application state

Each item can store callbacks using std::function, including onActivate, onHover, and onSelected. That’s how the menu connects to the rest of the interface: when a user activates an option, the menu triggers the stored callback, which can switch views, open a dialog, or run any application action.

User navigation is handled through handleNav(), which takes NavEvent values like Up, Down, Home, End, Activate, and Back. This maps input into selection movement and activation, and it respects whether the menu is open. Finally, buildRenderModel() produces a simplified list of RenderItem objects that the GUI can render directly, keeping rendering separate from logic. So overall, this Menu class manages menu state and interaction, while the GUI uses it to display actions cleanly

**Vivian (ErrorManager):**  
This is the ErrorManager project. This C++ library built on top of Qt gives any application a centralized way to report warnings, errors, and fatal conditions.

* C++ library built on Qt  
* Gives any application a centralized way to report warnings, errors, and fatal conditions

ErrorManager.hpp

* Defines the ErrorManager class  
* Two enums:  
* Severity (Warning, Error, Fatal): represents how serious a problem is  
* OutputMode (Terminal, GUI, Web, Custom): describes the runtime environment so messages can be adapted accordingly  
* ErrorHandler type: a std::function taking a message string and a severity level, allowing callers to plug in custom behavior

ErrorManager.cpp

* Three main public methods: reportWarning, reportError, reportFatal  
  * Each validates the message is non-empty — throws invalid\_argument if not  
  * Each calls the internal dispatch method  
* dispatch core logic: Checks severity level, finds the corresponding registered handler, and calls it. Falls back to defaultHandler if no custom handler is set  
  * defaultHandler prints a labeled prefix (\[Warning\], \[Error\], \[Fatal\]) to std::cerr  
* Fatal errors always throw std::runtime\_error after dispatching. This is intentional as fatal means execution should not continue

TestErrorManager.cpp

* Unit tests using the Qt Test framework. There are three tests, one per severity level:  
  * Empty message → correct exception thrown  
  * Valid message → no crash  
  * Fatal → std::runtime\_error thrown, matching dispatch behavior

ErrorManager is a comprehensive class to route all your application errors, with configurable handlers per severity level and a safe default behavior.

**Madi (ImageGrid):**  
This is the ImageGrid class. The purpose of this class is to simplify the drawing of the world background. Given a world class that derives from the base world class, such as MazeWorld, it will render a grid of specific tiles that correspond with certain characters. For example, it can replace a space character (‘ ‘) with a road tile, since we’re thinking traffic sim.

ImageGrid has four main member functions: LoadImages, MapImages, RenderGrid, and SetSceneAndView.

First, LoadImages takes a vector of QStrings (because that is what is used for QPixmaps) that contains the image paths, and it loops through it, creating a QPixmap of the image found at that location, and adds that pixmap to the image list. 

Next, MapImages maps the images we just loaded to their corresponding cell type. It gets the underlying grid from the world and its cell types, makes sure that the number of unique tile images and cell types are the same, and maps the images to the cell types by their ID in the same order the cell types were added. So in MazeWorld, the floor\_id is first, followed by the wall\_id, so the floor will be green and the walls will be blue.

Those are both used in the constructor to ensure you don’t make an ImageGrid without successfully loading and mapping images.

Next, RenderGrid is super simple. It loops over each cell in the grid and gets the cell id at that location. Find the corresponding tile image, and add it to the scene. It then sets the position with regard to the tile’s dimensions so that it renders the images next to each other instead of something unexpected, like overlapping.

Lastly, SetSceneAndView is in charge of setting the scene rectangle dimensions and displaying the view. The scene is the container for QGraphicsItems, like the pixmap items used to render the grid, and the view allows us to display the contents of the scene. The rect defines the extent of the scene and is used by the view to determine the area of the window.

I used a MazeWorld in my example, and you can see the grid defined here. I’ve specified the tileSize and imagePaths, and let’s go ahead and run it. And as you can see here, the floor cells are green tiles and the wall cells are blue tiles.

