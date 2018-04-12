### Environment constants

### general build targets
NET_SEV_PATH := communication
MOV_SEV_PATH := move_server

all:
	@echo "|---------------------------------------------------------|"
	@echo "|                  START MAKE FILE                        |"
	@echo "|---------------------------------------------------------|"
	$(MAKE) all -e -C $(NET_SEV_PATH)
	$(MAKE) all -e -C $(MOV_SEV_PATH)
##	cp local_conf.json ./exec
	mv $(NET_SEV_PATH)/bin/communication ./exec
	mv $(MOV_SEV_PATH)/bin/move_server   ./exec
	@echo "|---------------------------------------------------------|"
	@echo "|                  OK!                                    |"
	@echo "|---------------------------------------------------------|"
clean:
	@echo "|---------------------------------------------------------|"
	@echo "|                  START CLEAN FILE                       |"
	@echo "|---------------------------------------------------------|"
	$(MAKE) clean -e -C $(NET_SEV_PATH)
	$(MAKE) clean -e -C $(MOV_SEV_PATH)
	rm -f exec/communication
	rm -f exec/move_server
##	rm -f exec/monitor
	@echo "|---------------------------------------------------------|"
	@echo "|                  OK!                                    |"
	@echo "|---------------------------------------------------------|"
### EOF