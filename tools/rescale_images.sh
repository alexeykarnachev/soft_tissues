#!/bin/bash

# Ensure the script is run with three arguments
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <input_directory> <output_directory> <scale_factor>"
    exit 1
fi

# Get the arguments
input_directory="$1"
output_directory="$2"
scale_factor="$3"

# Check if the output directory exists; if not, create it
if [ -d "$output_directory" ]; then
    echo "Error: Output directory '$output_directory' already exists."
    exit 1
else
    mkdir -p "$output_directory"
fi

# Find all PNG files in the input directory and process them
find "$input_directory" -type f -name '*.png' | while read -r image; do
    # Compute the relative path from the input directory
    relative_path="${image#${input_directory}}"
    echo "Relative Path: $relative_path" 
    
    # Compute the output path
    output_path="$output_directory/$relative_path"
    
    # Create the directory structure in the output directory
    mkdir -p "$(dirname "$output_path")"
    
    # Create a temporary file to store the resized image
    temp_image="${output_path}.tmp"
    
    # Rescale the image using ImageMagick's 'convert' command
    convert "$image" -strip -resize "${scale_factor}%" "$temp_image"
    
    # Move the resized image to the output directory
    mv "$temp_image" "$output_path"
    
    echo "Rescaled $image to $output_path"
done
