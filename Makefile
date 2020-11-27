

SUBDIRS = axis_switch csc h-resampler h-scaler v-scaler v-resampler vprocss xgpio test 

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ 
