NCA_SHARED_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
NCA_SHARED_INCLUDE_DIRS := $(NCA_SHARED_DIR)include
NCA_SHARED_SOURCES := \
	$(NCA_SHARED_DIR)src/nca_protocol.c \
	$(NCA_SHARED_DIR)src/nca_interactive.c \
	$(NCA_SHARED_DIR)src/nca_topics.c \
	$(NCA_SHARED_DIR)src/nca_device.c
