all:
	$(MAKE) -C protobuf && $(MAKE) -C server && $(MAKE) -C client

clean:
	cd server && make clean && cd ../client && make clean && cd ../protobuf && make clean
