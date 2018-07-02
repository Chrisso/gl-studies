
all: land_shallow_topo_2048.jpg 2k_stars.jpg

# via https://visibleearth.nasa.gov/view_cat.php?categoryID=1484
land_shallow_topo_2048.jpg:
	curl -o $@ -L https://eoimages.gsfc.nasa.gov/images/imagerecords/57000/57752/land_shallow_topo_2048.jpg

# via https://www.solarsystemscope.com/textures/
2k_stars.jpg:
	curl -o $@ -L https://www.solarsystemscope.com/textures/download/2k_stars.jpg

clean:
	-rm land_shallow_topo_2048.jpg
