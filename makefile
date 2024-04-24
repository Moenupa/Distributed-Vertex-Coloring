all: ./src/coloring.cpp
#	gcc ./src/utils/graphio.c -c -O3
#	gcc ./src/utils/mmio.c -c -O3
	g++ ./src/coloring.cpp -c -O2 -fopenmp -std=c++20
	g++ -o coloring coloring.o ./src/utils/mmio.c ./src/utils/graphio.c -O2 -fopenmp -std=c++20

.PHONY: all clean \
		extract peek purge purgebin purgemtx purgeall \
		nlpkkt80 nlpkkt120 nlpkkt240

extract:
	tar -xzvf ./data/*.tar.gz -C ./data/
peek:
	du -sh ./data/*/*

# purge = remove (unwanted) data
purge:
	-@rm -vf ./data/*.tar.gz
purgebin:
	-@rm -vf ./data/*/*.bin
purgemtx:
	-@rm -vf ./data/*/*.mtx
purgeall: purge purgemtx purgebin

nlpkkt80:
	mkdir -p data
	wget -P ./data/ https://sparse.tamu.edu/MM/Schenk/nlpkkt80.tar.gz
nlpkkt120:
	mkdir -p data 
	wget -P ./data/ https://sparse.tamu.edu/MM/Schenk/nlpkkt120.tar.gz
nlpkkt240:
	mkdir -p data 
	wget -P ./data/ https://sparse.tamu.edu/MM/Schenk/nlpkkt240.tar.gz

clean:
	-@rm -v coloring *.o