/**
* This file is part of the Fall 2026, CSE 498, section 2, course project.
* @brief A class that replaces certain cell types with corresponding images (ie. replaces '#' with a road tile)
**/

#pragma once

#include <vector>
#include <string>
#include "ImageManager.hpp"

class ImageGrid {
private:
    size_t mWidth;  // number of cells/tiles in each row of the grid
    size_t mHeight; // number of rows in the grid
    std::vector<std::string> mImageIds; // 2D grid to store image IDs

    ImageManager& mImageManager; // ref to ImageManager to retrieve images

public:
    /// disabled default constructor
    ImageGrid() = delete;

    /// disabled default copy constructor
    ImageGrid(const ImageGrid &) = delete;

    /// disbaled assignment operator
    void operator=(const ImageGrid &) = delete;

    ImageGrid(size_t width, size_t height, ImageManager& imageManager)
        : mWidth(width), mHeight(height), mImageManager(imageManager) {
        imageIds.resize(width * height); // initialize with empty image IDs
    }

    // set the image for a specific cell in the grid
    void SetTile(size_t x, size_t y, const std::string& imageId) {
        if (x < width && y < height) {
            imageIds[y * width + x] = imageId;
        }
    }

    // get the image ID for a specific cell in the grid
    // return empty string if out of bounds
    std::string GetTile(size_t x, size_t y) const {
        if (x < width && y < height) {
            return imageIds[y * width + x];
        }
        return "";
    }

    // get the width of the grid
    size_t GetWidth() const {
        return width;
    }

    // get the height of the grid
    size_t GetHeight() const {
        return height;
    }

    // clear the grid
    void Clear() {
    }
    
};