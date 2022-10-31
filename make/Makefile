all: sole

-include make/Flags.mk
-include make/Objects.mk
-include make/Targets.mk

MAKEFILES:=make/Flags.mk make/Objects.mk make/Targets.mk \
           make/Autodep.mk make/Deps.mk make/Objects.m4

$(TARGETS):
	@echo "CC -o" $@
	@$(CC) $(CFLAGS) $(CFLAGSADD) $^ -o $@

clean:
	@echo "RM " $(OBJECTS)
	@rm -fv $(OBJECTS)

clean-make:
	@echo "RM " $(MAKEFILES)
	@rm -rfv $(MAKEFILES)

clean-targets:
	@echo "RM " $(TARGETS)
	@rm -rfv $(TARGETS)

clean-tags:
	@echo "RM TAGS"
	@rm -fv TAGS

tags:
	@echo "CTAGS TAGS"
	@ctags --output-format=etags -f TAGS -R .

.PHONY: clean clean-tags clean-make clean-targets all tags
