#pragma once 

#include <iostream>
#include <string>
#include <unordered_map>
#include <stdexcept>


class ImageManager {
    private:
        std::unordered_map<std::string, std::string> mImages;
    public:
        // Constructor
        ImageManager();
        
        // Destructor
        ~ImageManager();

        void Load(std::string imageId, std::string path);

        std::string GetImage(std::string imageId) const;

        void AddToImages(std::string imageId, std::string path);

        bool HasImage(std::string imageId);

        void Remove(std::string imageId);

        void Clear();
};

