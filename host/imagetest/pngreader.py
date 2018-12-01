import png

reader = png.Reader(filename='images/test2.png')
w, h, pixels, metadata = reader.read_flat()

print "Width", w, "Height", h, "Pixels", pixels, "Metadata", metadata
