# Root convenience targets. The handout build still lives in sfslab/Makefile.

HANDOUT = sfslab
DIST = sfslab-handout.tar
DIST_MTIME = 2024-01-01 00:00Z

.PHONY: all
all:
	$(MAKE) -C $(HANDOUT)

.PHONY: test
test:
	$(MAKE) -C $(HANDOUT) test

.PHONY: grade
grade:
	$(MAKE) -C $(HANDOUT) grade

.PHONY: smoke
smoke:
	$(MAKE) -C $(HANDOUT) smoke

.PHONY: check
check:
	$(MAKE) -C $(HANDOUT) check

.PHONY: trace-check
trace-check:
	$(MAKE) -C $(HANDOUT) trace-check

.PHONY: trace-list
trace-list:
	$(MAKE) -C $(HANDOUT) trace-list

.PHONY: handout-check
handout-check:
	$(MAKE) -C $(HANDOUT) handout-check

.PHONY: starter-check
starter-check:
	$(MAKE) -C $(HANDOUT) starter-check

.PHONY: clean
clean:
	$(MAKE) -C $(HANDOUT) clean

.PHONY: dist
dist:
	$(MAKE) -C $(HANDOUT) clean
	$(MAKE) -C $(HANDOUT) handout-check
	$(MAKE) -C $(HANDOUT) starter-check
	@if tar --version 2>/dev/null | grep -qi 'gnu tar'; then \
	  tar --sort=name --mtime='$(DIST_MTIME)' \
	    --owner=0 --group=0 --numeric-owner -cf $(DIST) $(HANDOUT); \
	else \
	  tar -cf $(DIST) $(HANDOUT); \
	fi
