*Will continue to be updated through Friday to refelct changes made from peer reviews.*

## ErrorManager
ErrorManager - A Qt-based C++ component that provides centralized error reporting with configurable severity levels and output modes.
1) Class Description
The goal of the ErrorManager class is to serve as a single, unified point of error handling for the entire GUI Interface system. Rather than having individual classes scatter ad-hoc print statements or crash unpredictably, any class in the project can call into ErrorManager to report problems at the appropriate severity level. When fully implemented, ErrorManager should be able to receive an error message and severity (Warning, Error, or Fatal), decide how to surface that information based on the current output mode (Terminal for debugging, GUI popup via QMessageBox for user-facing deployment, etc.), and either continue execution, log the issue, or terminate gracefully depending on severity.

## ImageManager
ImageManager - An object that handles image assets by name (ID) and path, and loads images into QPixmap objects.

The goal of this class is to have images ready for background, sprites, road signs, maps, ect. When it is fully implemented it should be able to pull up images and load them onto the screen when requested. The images will be loading into a QPixmap to ensure quick image access.

    std::unordered_map: could be used to store the images
    std::find: to find images
    std::exception: for any issues with loading in pictures
    std::out_of_range: for imageID’s not found
    std::unexpected: for imageID’s not found

## ImageGrid
ImageGrid is used to display a grid of images corresponding to certain characters using cell type IDs.

    void MapImages() - Map images to their cell type
    void RenderGrid() - Add the images to the scene in the correct location
    void SetSceneAndView(QGraphicsView &view) - Render the scene using the provided view

ImageGrid will mainly be used by Worlds to render a background for other items to be drawn on top of.

## Menu
Goal: dynamic menu model storing a collection of meny items managing their state and interactions, supports callback, predicate runtime state and keyboard navigation for GUI display.

Model:
- each item stores key, label, id
- base state flags: enabled, visible, selected
- callback: onActivate (invoked when activated & allowed), onHover (invoked when hovered and allowed), onSelected (invoked on selection only)
- navigation: handleNav(event) processes navigation events:
    - Up/Down/Left/Right - move selection among selectable items
    - Activate - activate current selected item
    - Back/Home/End - back action, optional, home and end jump to first/last selectable item
- predicates: provide dynamic runtime state 
- buildRenderModel - returns vector of RenderItem entries

## Text

Main concept with the Text class to make it usable with the other classes is to incorporate a category section. The idea of the category section is to make it easier for the programmer using the Text class to reuse the same text without having to write in the specifications everytime. 

For example, let’s say the user wants to display to the screen that the user is infected, the text would look the same, for example it could be a infectious green color and all capital, etc. But the user would have to specify each of the parameters everytime to reuse the text. 

The GUI Classes and Standard Library classes used will include: 
- Std::string
- QColor
- QFont
- QGraphicsTextItems
- QGraphicsScene
- QGraphicsView

Some new functions would include 
- GetCategory/SetCategory

There could be a way for the user to override one of the parameters (for example maybe the user wants the infected text to display in lowercase, so the user could still use the infected category but then override the specific uppercase category to be false). 

Challenges would include being able to coordinate and determine how many different categories would be needed, and trying to implement the override feature. 

The main groups/classes collaborating with: 

- Menu
- ErrorManager
- WebTextbox
- DataLog
- World Group (there was some talk about creating a dashboard to display data, so being able to display that data would involve text)