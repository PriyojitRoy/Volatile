import sys
from pathlib import Path
import argparse

try:
    from PIL import Image
except ImportError:
    print("Pillow library is required. Please install it using: pip install Pillow")
    sys.exit(1)

def image_to_ascii(image_path, width=80):
    try:
        img = Image.open(image_path).convert('RGB')
    except Exception as e:
        print(f"Error opening image: {e}")
        sys.exit(1)
        
    # Calculate height to maintain aspect ratio (terminal fonts are typically 2x taller than wide)
    w, h = img.size
    aspect_ratio = h / w
    height = int(width * aspect_ratio * 0.5)
    
    img = img.resize((width, height), Image.LANCZOS)
    
    # ASCII characters mapped by luminance (dark to light)
    chars = "@%#*+=-:. "
    
    cpp_string = ""
    for y in range(height):
        cpp_string += '    "'
        for x in range(width):
            r, g, b = img.getpixel((x, y))
            # Calculate luminance for character selection
            luminance = int(0.299 * r + 0.587 * g + 0.114 * b)
            char_idx = int(luminance / 256 * len(chars))
            
            # Escape double quotes or backslashes
            char = chars[char_idx]
            if char == '"':
                char = '\\"'
            elif char == '\\':
                char = '\\\\'
            
            # Using 24-bit ANSI color codes
            cpp_string += f"\\x1b[38;2;{r};{g};{b}m{char}"
        
        # Reset color at the end of each line and add newline
        cpp_string += "\\x1b[0m\\n\"\n"
        
    return cpp_string

def generate_header(image_path, output_path):
    ascii_art = image_to_ascii(image_path, width=70)
    
    header_content = f"""#ifndef VOLATILE_LOGO_HPP
#define VOLATILE_LOGO_HPP

namespace VEngine {{
namespace Constants {{

constexpr const char* ENGINE_GITHUB_LINK = "https://github.com/PriyojitRoy/Volatile";
constexpr const char* ENGINE_LOGO = 
{ascii_art};

}} // namespace Constants
}} // namespace VEngine

#endif // VOLATILE_LOGO_HPP
"""
    
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(header_content)
    
    print(f"Successfully generated {output_path}")

if __name__ == "__main__":
    project_root = Path(__file__).resolve().parent.parent
    image_path = project_root / "VCE_logo.png"
    output_path = project_root / "include" / "core" / "Logo.hpp"
    
    if not image_path.exists():
        print(f"Error: Could not find image at {image_path}")
        sys.exit(1)
        
    # Ensure include directory exists
    output_path.parent.mkdir(parents=True, exist_ok=True)
    
    generate_header(image_path, output_path)
