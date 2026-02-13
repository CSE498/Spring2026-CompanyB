#include "ImageManager.hpp"

// Constructor
ImageManager::ImageManager() {
    // Initialize empty image map
    std::cout << "ImageManager initialized" << std::endl;
}

// Destructor
ImageManager::~ImageManager() {
    // Clear all images on destruction
    Clear();
    std::cout << "ImageManager cleared" << std::endl;
}

// Load an image from file and store it in the map
void ImageManager::Load(std::string imageId, std::string path) {
    if (imageId.empty()) {
        std::cerr << "Error: Image ID is empty" << std::endl;
        return;
    }
    
    if (path.empty()) {
        std::cerr << "Error: Path cannot be empty" << std::endl;
        return;
    }

    mImages[imageId] = path;
    std::cout << "Successfully loaded image '" << imageId << "' from " << path << std::endl;

}

// Retrieve an image path by ID
std::string ImageManager::GetImage(std::string imageId) const {
    auto it = mImages.find(imageId);
    
    // if (it == mImages.end()) {
    //     //insert error cannot fine image here
    // }
    
    return it->second;
}

// Add an image to the collection directly (for preloaded images)
void ImageManager::AddToImages(std::string imageId, std::string path) {
    if (imageId.empty() || path.empty()) {
        std::cerr << "Error: Cannot add image with empty ID or empty path" << std::endl;
        return;
    }
    
    mImages[imageId] = path;
    std::cout << "Added image '" << imageId << "' to manager" << std::endl;
}

// Check if image exists in manager
bool ImageManager::HasImage(std::string imageId) {
    return mImages.find(imageId) != mImages.end();
}

// Remove a specific image from the manager
void ImageManager::Remove(std::string imageId) {
    auto it = mImages.find(imageId);
    
    // if (it == mImages.end()) {
    //     //insert error message here
    //     return;
    // }
    
    mImages.erase(it);
}

// Clear all images from the manager
void ImageManager::Clear() {
    if (!mImages.empty()) {
        std::cout << mImages.size() << "Images cleared" << std::endl;
        mImages.clear();
    }
}
