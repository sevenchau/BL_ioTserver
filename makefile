### Environment constants

### general build targets
NET_SEV_PATH := communication
## MONITOR_PATH := monitor

all:
	@echo "|---------------------------------------------------------|"
	@echo "|                  START MAKE FILE                        |"
	@echo "|---------------------------------------------------------|"
	$(MAKE) all -e -C $(NET_SEV_PATH)
##	$(MAKE) all -e -C $(MONITOR_PATH)
##	cp local_conf.json ./exec
	cp $(NET_SEV_PATH)/bin/communication ./exec
##	cp $(MONITOR_PATH)/monitor ./exec
	@echo "|---------------------------------------------------------|"
	@echo "|                  OK!                                    |"
	@echo "|---------------------------------------------------------|"
clean:
	@echo "|---------------------------------------------------------|"
	@echo "|                  START CLEAN FILE                       |"
	@echo "|---------------------------------------------------------|"
	$(MAKE) clean -e -C $(NET_SEV_PATH)
##	$(MAKE) clean -e -C $(MONITOR_PATH)
	rm -f exec/communication
##	rm -f exec/terminal
##	rm -f exec/monitor
	@echo "|---------------------------------------------------------|"
	@echo "|                  OK!                                    |"
	@echo "|---------------------------------------------------------|"
### EOF