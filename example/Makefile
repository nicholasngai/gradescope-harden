TARGET = autograder.zip

SRCS = \
	setup.sh \
	run_autograder \
	requirements.txt \
	run_tests.py \
	$(wildcard tests/*.py)

$(TARGET): $(SRCS)
	zip -r $@ $^

.PHONY: clean
clean:
	rm -rf $(TARGET)
