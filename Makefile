

SUBDIRS = axis_switch csc h-resampler h-scaler v-scaler v-resampler vprocss xgpio test 

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ 

install:
	$(info D is ${DESTDIR})
	$(info B is ${BUILDDIR})
	mkdir -p ${DESTDIR}/usr/bin
	cp ${BUILDDIR}/test/vpss_app ${DESTDIR}/usr/bin
