# Root convenience targets. The handout build still lives in sfslab/Makefile.

HANDOUT = sfslab
DIST = sfslab-handout.tar

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

.PHONY: trace-check
trace-check:
	$(MAKE) -C $(HANDOUT) trace-check

.PHONY: handout-check
handout-check:
	$(MAKE) -C $(HANDOUT) handout-check

.PHONY: clean
clean:
	$(MAKE) -C $(HANDOUT) clean

.PHONY: dist
dist:
	$(MAKE) -C $(HANDOUT) handout-check
	tar -cf $(DIST) $(HANDOUT)
