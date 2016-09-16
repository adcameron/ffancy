CC = gcc
CFLAGS = -Wall -Werror -lm

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

ffancy : ffancy.o dataarray.o ffa.o ffadata.o mad.o  metric1.o metric2.o metric3.o metric4.o metric5.o metric7.o metric8.o paddedarray.o power2resizer.o stringsequal.o whitenoise.o runningmedian.o
	$(CC) $(CFLAGS) ffancy.o dataarray.o ffa.o ffadata.o mad.o metric1.o metric2.o metric3.o metric4.o metric5.o metric7.o metric8.o  paddedarray.o power2resizer.o stringsequal.o whitenoise.o runningmedian.o -o $@

#ffatester : ffatester.o dataarray.o ffa.o ffadata.o mad.o metric5.o paddedarray.o power2resizer.o whitenoise.o
#	$(CC) $(CFLAGS) dataarray.o ffatester.o ffa.o ffadata.o mad.o metric5.o paddedarray.o power2resizer.o whitenoise.o -o $@

#mftester : mftester.o ffa.o ffadata.o mad.o metric5.o stringsequal.o paddedarray.o power2resizer.o dataarray.o whitenoise.o
#	$(CC) $(CFLAGS) mftester.o ffa.o ffadata.o metric5.o stringsequal.o paddedarray.o power2resizer.o dataarray.o whitenoise.o -o $@

#madtester : madtester.o ffadata.o mad.o stringsequal.o
#	$(CC) $(CFLAGS) madtester.o ffadata.o mad.o stringsequal.o -o $@

progeny : progeny.o whitenoise.o stringsequal.o
	$(CC) $(CFLAGS) progeny.o whitenoise.o stringsequal.o -o $@

prostat : prostat.o stats.o stringsequal.o
	$(CC) $(CFLAGS) prostat.o stats.o stringsequal.o -o $@

metrictester : metrictester.o stringsequal.o metric8.o metric1.o metric2.o metric4.o metric5.o metric3.o metric7.o mad.o ffadata.o
	$(CC) $(CFLAGS) metrictester.o stringsequal.o metric8.o metric1.o metric2.o metric4.o metric5.o metric3.o metric7.o mad.o ffadata.o -o $@

add_periodograms : add_periodograms.o stringsequal.o
	$(CC) $(CFLAGS) add_periodograms.o stringsequal.o -o $@

ffa2best : ffa2best.o stringsequal.o
	$(CC) $(CFLAGS) ffa2best.o stringsequal.o -o $@

#snr2sigma : snr2sigma.o dcdflib.o stringsequal.o ipmpar.o
#	$(CC) $(CFLAGS) snr2sigma.o dcdflib.o stringsequal.o ipmpar.o -o $@

clean :
	rm *.o
