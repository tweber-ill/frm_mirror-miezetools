.PHONY: clean All

All:
	@echo "----------Building project:[ cattus - Debug ]----------"
	@$(MAKE) -f  "cattus.mk"
clean:
	@echo "----------Cleaning project:[ cattus - Debug ]----------"
	@$(MAKE) -f  "cattus.mk" clean
