
all: land_shallow_topo_2048.jpg

land_shallow_topo_2048.jpg:
	curl -o $@ -L https://eoimages.gsfc.nasa.gov/images/imagerecords/57000/57752/land_shallow_topo_2048.jpg

clean:
	-rm land_shallow_topo_2048.jpg
