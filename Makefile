#
# Makefile for the touchscreen drivers.
#

# Each configuration option enables a list of files.


obj-$(CONFIG_SIS_OEMINBUF)	+= oeminbuf.o
obj-$(CONFIG_SIS_SISINPUT)	+= sisinput.o
obj-$(CONFIG_SIS_SISTRANS)	+= sistrans.o
obj-$(CONFIG_SIS_SISVAILD)	+= sisvalid.o
obj-$(CONFIG_SIS_TRACKING)	+= tracking_kalman.o
obj-$(CONFIG_SIS_WTRACE)	+= wtrace.o
obj-$(CONFIG_SIS_TRACKER)	+= tracker.o
