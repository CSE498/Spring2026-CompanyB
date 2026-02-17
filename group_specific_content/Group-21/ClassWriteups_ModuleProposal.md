# Group 21 Class Writeups and Module Proposal

1. A class description, including what its goals are, and what its high-level functionality should look like. This does not need to perfectly match the description you were given, but it should be in the same general spirit unless you confirm any changes with the instructors ahead of time.
2. A list of similar classes in the standard library that you should be familiar with or use to inform the functionality you will be developing.
3. A list of key functions that you plan to implement. This does not need to be an exhaustive list, but it should give a strong indication of how the class should be used.
4. A set of error conditions that you will be responsive to. For each, indicate if its source was (1) programmer error, such as invalid arguments to a function, which should be caught by an assert, (2) a potentially recoverable error, such as a resource limitation, which should trigger an exception, or (3) a user error, such as invalid input, which needs to simply return a special condition for the programmer to respond to (note: these can usually be handled with exceptions as well if you prefer.)
5. Any expected challenges that you will be facing, an especially any extra topics you need to learn about. This information will help me make sure to cover topics early in the course or else provide your group with extra guidance on the project.
6. A list of any other group’s C++ classes that you think you may want to coordinate with (e.g., to have a compatible interface).





### Class Writeups

##### ImageGrid

1. A grid of images (presumably from the ImageManager) that can be displayed as a unit. Simplifies the drawing a World background for the users. Typically agents and items would be layered on top of this ImageGrid.

* Displays images to a grid of tiles as a unit to create the background for a World. Agents and items will be layered on top of this ImageGrid.
* Stores a 2D tiled layout of images from the ImageManager
* Allows tile images to be reused rather than rendering one single large image
* Potentially only draws what is visible within the user's viewport (given there will exist a camera that moves with the player and the world is large)



2\.

* std::vector: can be used to store the grid data
* std::unordered\\\_map: can be used to map images to tile IDs
* std::optional: can be used to safely handle returning invalid tiles/coordinates



3\.

* ImageGrid
* setTile
* getTile
* checkInBounds
* draw
* getWidth
* getHeight
* getTileWidth
* getTileHeight
* worldToGridPos
* gridToWorldPos



4\.

* Out-of-bounds tile access: programmer error
* Invalid image ID: programmer error



5\.

* Rendering only tiles on screen
* Converting between grid and world coordinates



6\.

* StateGrid





##### Menu

1. Class that represents a graphical menu system, allowing users to interact with the application by selecting from a list of options.

* Each menu option associated with a function or call, triggered when user interacts with it
* Designed for navigation and interaction layer between user and the underlying application logic
* Provide flexible way for user actions, navigate between pages, views, and trigger application behavior 



2\.

* std::vector - used to order the ordered list of menu items
* std::string - used to represent menu item labels 
* std::optional - handle cases where a requested menu item does not exist



3\.

* Menu 
* addItem
* removeItem
* setItem/setItemEnabled
* getItem
* handleInput
* selectNext/selectPrevious
* Draw
* activateSelected



4\.

* Accessing a non existent menu item (programmer error)
* Accessing menu item with no callback (programmer error)
* Activating a disabled menu item (user error)



5\.

* Making sure Menu triggers callback without ruining logic
* Managing menu state - tracking the currently selected item, enabled/disabled states and the visibility of menu items



6\.

* ActionMap





##### Text

1. Main purpose is that the text class is in charge of any of the text and how it is displayed. The text class would take regular text and modify its display depending on what is needed, such as changing the color, font, or adding special styles such as bold or italics. The text class is specifically used by people writing the program in order to format their text for displaying it on the screen.



2\.

* std::string -> text is going to be a string, so the string class is the main class being used
* We have not determined our GUI package yet, but will most likely be different classes within the GUI library such as font, color, etc.



3\.

* displayText
* get/setFont
* get/setColor\*
* get/setBold
* get/setUnderline
* get/setItalics
* get/setGlowing
* get/setSize
* get/setString
* get/setUppercase
* get/setLowercase



* Another idea I have is that the program user enters in a category and it generically formats the text based off the category. For example, if we implement a point system where the user gains or loses points, the points would be displayed in the color green, generic font, all capital, etc. and the program would format it for the user by default instead of the program user having to type out all the features needed everytime. However this is not an idea that is completely set in place and is also dependent on what other groups think. But if this ends up getting implemented, additional functions would be get/setCategory.



4\.

* Inputting an invalid feature such as a font that does not exist (user error)
* Send message to user and ask user to enter in a valid font
* Forgetting to include a feature for the text class (user error)
* Send message to user of missing feature/s and ask user to enter in the missing feature/s
* User selects valid feature but program is unable to detect it (system error)
* Could re-prompt user and say system does not recognize feature (could be handled by ErrorManager)



5\.

* Working with the GUI libraries and figuring out how to implement the features with them
* Since the text feature will be used with multiple other groups and classes, making sure to implement it so that it can be used seamlessly between all the different groups/classes
* If implementing the Categories option, also ties into previous bullet, but coordinating with other groups to determine the features, as well as figuring out the most efficient way to implement the categories feature while also taking into consideration every single edge case such as overriding any of the category features



6\.

* WebTextbox (Group 22)
* DataLog (Group 20)



##### ImageManager

1. An object that handles image assets by name, pre-loading them (if needed) and displaying them on the screen as requested. The goal of this class is to have images ready for background, sprites, road signs, maps, ect. When it is fully implemented it should be able to pull up images and load them onto the screen when requested, and this might mean it has some images already loaded in for efficiency.



2\.

* std::vector: could be used to store the images 
* std::unordered\_map: could be used to store the images 
* std::find: to find images
* std::exception: for any issues with loading in pictures



3\.

* Load
* GetImage
* draw
* AddToImages



4\.

* Invalid image ID: programmer error
* Cannot load image: exception error



5\.

* I’m expecting that figuring out which images are needed will need cooperation with other groups



6\.

* Surface, if the images need to be loaded into the background
* DataLog if the images are to be tracked when they get loaded in





##### ErrorManager

1. The main purpose of the ErrorManager class is to provide a centralized way to handle errors and warnings that occur throughout the application. Instead of each class handling errors differently, ErrorManager allows errors to be reported in a consistent way while still being flexible depending on how the program is compiled or displayed.



2\. 

* std::exception – used as a base for recoverable or fatal errors
* std::runtime\\\_error – for general runtime error handling
* std::function – used to store customizable error-handling callbacks
* std::string – used to represent error messages
* std::ostream – for printing errors to output streams such as the console



3\.

* reportError
* reportWarning
* reportFatal
* setErrorHandler
* setWarningHandler
* setOutputMode
* clearHandlers



4\.

* Reporting an error with an invalid or empty message (programmer error)
* Triggering an action with no callback set (programmer error)
* User input does not map to any action (user error)
* Fatal errors that prevent the simulation from continuing (exception error)



5\.

* Designing the class so it works across multiple environments (terminal, GUI, web)
* Managing different levels of error severity in a clean and extensible way
* Allowing customization of error behavior without making the interface overly complex
* Coordinating with other groups to ensure consistent error reporting throughout the project



6\.

* Text / WebTextbox (Group 22) – formatting error messages for display
* ActionMap / InputHandler – handling invalid or unsupported user input
* DataLog (Group 20) – logging errors and warnings for debugging or analytics





### Module Proposal

##### Campus Commuting module

Agents can be students, faculty, and modes of transport (cars, buses, scooters, bikes, motorcycles). World can again be based off campus maps but have different word based obstacles like blocked roads (under construction), weather related issues, river in the middle of campus (minus points for going into the water), buses getting full, time crunches due to student schedules etc. The goal of the simulation is to find the safest and most efficient route from point A to point B. For data analytic purposes, the user should be able to click onto another page to view intuitive visualizations of the data collected from the simulation using the menu.

