#!/usr/bin/env python3
"""
HTML to Arduino PROGMEM Converter
Converts HTML files to compressed PROGMEM strings for Arduino projects.
"""

import re
import os
import sys
from pathlib import Path

#!/usr/bin/env python3
"""
HTML to Arduino PROGMEM Converter with Advanced Compression
Converts HTML files to highly compressed PROGMEM strings for Arduino projects.
"""

import re
import os
import sys
import gzip
import base64
from pathlib import Path

def advanced_minify_html(html_content):
    """
    Conservative HTML minification that completely preserves JavaScript content.
    """
    # Find all script tags and their content
    script_pattern = r'(<script[^>]*>)(.*?)(<\/script>)'
    scripts = re.findall(script_pattern, html_content, re.DOTALL)
    
    # Replace each script block with a unique placeholder
    temp_content = html_content
    for i, (start_tag, script_content, end_tag) in enumerate(scripts):
        placeholder = f"___SCRIPT_BLOCK_{i}___"
        full_script = start_tag + script_content + end_tag
        temp_content = temp_content.replace(full_script, placeholder, 1)
    
    # Now process the HTML without any script content
    html_content = temp_content
    
    # Remove HTML comments (but preserve conditional comments)
    html_content = re.sub(r'<!--(?!\s*\[if\s).*?-->', '', html_content, flags=re.DOTALL)
    
    # Remove extra whitespace between tags
    html_content = re.sub(r'>\s+<', '><', html_content)
    
    # Remove whitespace at the beginning and end of lines
    html_content = re.sub(r'^\s+|\s+$', '', html_content, flags=re.MULTILINE)
    
    # Remove empty lines
    html_content = re.sub(r'\n\s*\n', '\n', html_content)
    
    # Advanced CSS compression (safe since CSS is separate from JavaScript)
    def compress_css_block(match):
        css_content = match.group(1)
        
        # Remove CSS comments
        css_content = re.sub(r'/\*.*?\*/', '', css_content, flags=re.DOTALL)
        
        # Remove spaces around CSS operators and punctuation
        css_content = re.sub(r'\s*([{}:;,>+~()=])\s*', r'\1', css_content)
        
        # Compress color values (e.g., #ffffff -> #fff)
        css_content = re.sub(r'#([0-9a-fA-F])\1([0-9a-fA-F])\2([0-9a-fA-F])\3', r'#\1\2\3', css_content)
        
        # Remove unnecessary semicolons before closing braces
        css_content = re.sub(r';}', '}', css_content)
        
        # Remove spaces around decimal points in numbers
        css_content = re.sub(r'(\d)\s*\.\s*(\d)', r'\1.\2', css_content)
        
        # Compress 0 values (remove units from 0 values)
        css_content = re.sub(r'\b0(px|em|pt|%|rem|vh|vw|ex|ch|cm|mm|in|pc)\b', '0', css_content)
        
        # Remove extra spaces
        css_content = re.sub(r'\s+', ' ', css_content)
        css_content = css_content.strip()
        
        return f'<style>{css_content}</style>'
    
    html_content = re.sub(r'<style[^>]*>(.*?)</style>', compress_css_block, html_content, flags=re.DOTALL)
    
    # Limited HTML attribute compression - only for simple attributes
    html_content = re.sub(r'=\s*"([a-zA-Z0-9_-]+)"\s*(?=[\s>])', r'=\1 ', html_content)
    
    # Remove spaces around = in attributes
    html_content = re.sub(r'\s*=\s*', '=', html_content)
    
    # Final HTML cleanup
    html_content = re.sub(r'[ \t]+', ' ', html_content)
    html_content = html_content.strip()
    
    # Now restore the script blocks with MINIMAL processing (only comment removal)
    for i, (start_tag, script_content, end_tag) in enumerate(scripts):
        placeholder = f"___SCRIPT_BLOCK_{i}___"
        
        # ONLY remove comments from JavaScript, preserve everything else completely
        processed_script = script_content
        
        # Remove single-line comments - but be careful not to remove // inside strings
        # Only remove // that are at the start of a line or preceded by whitespace
        processed_script = re.sub(r'(?:^|\s)//[^\r\n]*', '', processed_script, flags=re.MULTILINE)
        
        # Remove multi-line comments - but be careful not to remove /* inside strings  
        processed_script = re.sub(r'/\*.*?\*/', '', processed_script, flags=re.DOTALL)
        
        # Only remove leading and trailing whitespace from each line
        lines = processed_script.split('\n')
        processed_lines = [line.strip() for line in lines]
        processed_script = '\n'.join(processed_lines)
        
        # Remove multiple consecutive empty lines
        processed_script = re.sub(r'\n\s*\n\s*\n+', '\n\n', processed_script)
        processed_script = processed_script.strip()
        
        # Restore the complete script block
        full_script_block = start_tag + processed_script + end_tag
        html_content = html_content.replace(placeholder, full_script_block)
    
    return html_content

def generate_gzip_progmem_string(html_content, variable_name, use_gzip=True):
    """
    Generate a PROGMEM string with optional GZIP compression.
    """
    minified = advanced_minify_html(html_content)
    
    original_size = len(html_content)
    minified_size = len(minified)
    
    if use_gzip:
        # Compress with gzip
        compressed_data = gzip.compress(minified.encode('utf-8'), compresslevel=9)
        compressed_size = len(compressed_data)
        
        # Convert to base64 for embedding (optional, or we could use hex)
        # For Arduino, we'll use hex representation
        hex_data = compressed_data.hex()
        
        # Split hex data into lines of reasonable length (32 bytes per line = 64 hex chars)
        hex_lines = [hex_data[i:i+64] for i in range(0, len(hex_data), 64)]
        
        print(f"{variable_name} (with GZIP):")
        print(f"  Original size: {original_size} bytes")
        print(f"  Minified size: {minified_size} bytes")
        print(f"  Compressed size: {compressed_size} bytes")
        print(f"  Total compression: {(1 - compressed_size / original_size) * 100:.1f}%")
        
        # Generate the PROGMEM declaration with compressed data
        progmem_string = f'''// Compressed {variable_name} (GZIP)
const unsigned char {variable_name}_compressed[] PROGMEM = {{
'''
        
        for i, line in enumerate(hex_lines):
            bytes_in_line = [f'0x{line[j:j+2]}' for j in range(0, len(line), 2)]
            progmem_string += '  ' + ', '.join(bytes_in_line)
            if i < len(hex_lines) - 1:
                progmem_string += ','
            progmem_string += '\n'
        
        progmem_string += f'}};'
        progmem_string += f'\nconst size_t {variable_name}_compressed_size = {compressed_size};\n'
        progmem_string += f'const size_t {variable_name}_uncompressed_size = {minified_size};\n\n'
        
    else:
        minified_compression = (1 - minified_size / original_size) * 100
        
        print(f"{variable_name} (minified only):")
        print(f"  Original size: {original_size} bytes")
        print(f"  Minified size: {minified_size} bytes")
        print(f"  Compression: {minified_compression:.1f}%")
        
        # Standard PROGMEM string
        progmem_string = f'const char {variable_name}[] PROGMEM = R"rawliteral(\n{minified}\n)rawliteral";\n\n'
    
    return progmem_string

def generate_decompression_helper():
    """
    Generate helper code for GZIP decompression on Arduino.
    """
    return '''
// GZIP decompression helper function
// NOTE: You'll need to install ArduinoHttpClient or ESP32 built-in gzip support
// For ESP32: #include "esp_gzip.h" or use built-in WebServer gzip support

#ifdef ESP32
#include <Update.h>
String decompressGzip(const unsigned char* compressed_data, size_t compressed_size, size_t uncompressed_size) {
    // For ESP32, you can use built-in GZIP support
    // This is a placeholder - actual implementation depends on your specific needs
    
    // Alternative: serve compressed directly and let browser decompress
    // server.sendHeader("Content-Encoding", "gzip");
    // server.send_P(200, "text/html", (const char*)compressed_data, compressed_size);
    return "";
}
#endif

'''

def main():
    # Get the current directory (where the script is located)
    script_dir = Path(__file__).parent
    
    # Define file paths
    index_html_path = script_dir / "index.html"
    order_html_path = script_dir / "order.html"
    frontend_h_path = script_dir / "Frontend.h"
    
    # Check if input files exist
    if not index_html_path.exists():
        print(f"Error: {index_html_path} not found!")
        return 1
    
    if not order_html_path.exists():
        print(f"Error: {order_html_path} not found!")
        return 1
    
    # Ask user about compression options
    print("Choose compression method:")
    print("1. Advanced minification only (default)")
    print("2. Advanced minification + GZIP compression")
    choice = input("Enter choice (1-2) or press Enter for default: ").strip()
    
    use_gzip = choice == "2"
    
    try:
        # Read HTML files
        print("\nReading HTML files...")
        with open(index_html_path, 'r', encoding='utf-8') as f:
            index_html = f.read()
        
        with open(order_html_path, 'r', encoding='utf-8') as f:
            order_html = f.read()
        
        print("Generating PROGMEM strings...")
        
        # Generate PROGMEM strings
        index_progmem = generate_gzip_progmem_string(index_html, "indexPage", use_gzip)
        order_progmem = generate_gzip_progmem_string(order_html, "orderPage", use_gzip)
        
        # Generate Frontend.h content
        if use_gzip:
            frontend_content = f"""#ifndef FRONTEND_H
#define FRONTEND_H

#include <Arduino.h>

// Auto-generated HTML pages for Arduino web server
// Generated by html_to_progmem.py with advanced compression

#define COMPRESSED_PAGES  // Pages are GZIP compressed

{index_progmem}{order_progmem}"""
        else:
            frontend_content = f"""#ifndef FRONTEND_H
#define FRONTEND_H

#include <Arduino.h>

// Auto-generated HTML pages for Arduino web server
// Generated by html_to_progmem.py with advanced compression

{index_progmem}{order_progmem}"""
        
        if use_gzip:
            frontend_content += generate_decompression_helper()
        
        frontend_content += "#endif // FRONTEND_H\n"
        
        # Write to Frontend.h
        with open(frontend_h_path, 'w', encoding='utf-8') as f:
            f.write(frontend_content)
        
        print(f"\nFrontend.h has been generated successfully!")
        print(f"Location: {frontend_h_path}")
        print(f"Total header size: {len(frontend_content)} bytes")
        
        if use_gzip:
            print("\nNOTE: GZIP compression requires decompression on the Arduino.")
            print("For ESP32, you can serve compressed data directly:")
            print('  server.sendHeader("Content-Encoding", "gzip");')
            print('  server.send_P(200, "text/html", (const char*)indexPage_compressed, indexPage_compressed_size);')
        
        return 0
        
    except Exception as e:
        print(f"Error: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())