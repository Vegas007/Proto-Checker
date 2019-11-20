#! /usr/bin/env python
# -*- coding: utf-8 -*-
__name__ = "ProtoChecker"
__author__ = "VegaS"
__date__ = "2019-11-18"
__version__ = "0.0.1"

import sys
import os

# Set the base color of command prompt
os.system("color 00")

# Set the print method
TraceFormat = lambda arg: sys.stdout.write(arg + "\n")

#################################################
## Builtin translations
#################################################
COLOR_GREEN = "\033[92m{}\033[00m"
COLOR_RED = "\033[91m{}\033[00m"
COLOR_GRAY = "\033[37m{}\033[00m"

TRANSLATE_DICT = {
	"FILE_EMPTY": COLOR_RED.format("File {} is empty."),
	"FILE_NAME": COLOR_GRAY.format("Reading file: {}"),
	"FILE_OK": COLOR_GREEN.format("\tOK"),
	"FILE_TOTAL_LINES": COLOR_RED.format("\tTotal lines: {}"),
	"FILE_DUPLICATE_LINE": COLOR_RED.format("\tDuplicated itemVnum: {} at line: {}"),
	"FILE_COMPARING_LINE": COLOR_RED.format("\tMissing itemVnum: {}"),
}

for localeName, localeValue in TRANSLATE_DICT.items():
	locals().update({localeName: localeValue})

#################################################
## ProtoChecker
#################################################
class ProtoChecker:
	ITEM_PROTO_FILE_NAME = 'item_proto.txt'
	ITEM_NAMES_FILE_NAME = 'item_names.txt'

	MOB_PROTO_FILE_NAME = 'mob_proto.txt'
	MOB_NAMES_FILE_NAME = 'mob_names.txt'

	def __init__(self):
		self.itemProtoDict = self.load_file(self.ITEM_PROTO_FILE_NAME)
		self.itemNamesDict = self.load_file(self.ITEM_NAMES_FILE_NAME)

		self.mobProtoDict = self.load_file(self.MOB_PROTO_FILE_NAME)
		self.mobNamesDict = self.load_file(self.MOB_NAMES_FILE_NAME)

	@staticmethod
	def load_file(fileName, token='\t'):
		fileExists = os.path.exists(fileName)
		if fileExists:
			if os.path.getsize(fileName) == 0:
				TraceFormat(FILE_EMPTY.format(fileName))
				sys.exit(0)

		fileData = []
		if fileExists:
			with open(fileName, 'r') as file:
				for line in file.readlines():
					if token in line:
						line = line.split(token)[0]
						if line.isdigit():
							fileData.append(line)

		return {'name': fileName, 'data': fileData, 'exists': fileExists}

	@staticmethod
	def find_duplicate(file):
		if not file["exists"]:
			return

		TraceFormat(FILE_NAME.format(file["name"]))

		duplicateSet = set([itemVnum for itemVnum in file["data"] if file["data"].count(itemVnum) > 1])
		for itemVnum in duplicateSet:
			TraceFormat(FILE_DUPLICATE_LINE.format(itemVnum, len(file["data"]) - file["data"][-1::-1].index(itemVnum) + 2))

		TraceFormat(FILE_TOTAL_LINES.format(duplicateSet.__len__()) if duplicateSet else FILE_OK)

	@staticmethod
	def compare(firstFile, secondFile):
		if not firstFile['exists'] or not secondFile['exists']:
			return

		TraceFormat(FILE_NAME.format(secondFile['name']))

		differenceSet = set(firstFile['data']).difference(secondFile['data'])
		if not differenceSet:
			TraceFormat(FILE_OK)
			return

		for itemVnum in differenceSet:
			TraceFormat(FILE_COMPARING_LINE.format(itemVnum))

		TraceFormat(FILE_TOTAL_LINES.format(differenceSet.__len__()))

	def run(self):
		TraceFormat("###### START_CHECKING_FOR_DUPLICATE ######")
		for file in (self.itemProtoDict, self.itemNamesDict, self.mobProtoDict, self.mobNamesDict):
			self.find_duplicate(file)
		TraceFormat("###### END_CHECKING_FOR_DUPLICATE ######\n")

		TraceFormat("###### START_COMPARING ######")
		for files in ((self.itemProtoDict, self.itemNamesDict), (self.mobProtoDict, self.mobNamesDict)):
			for firstFile, secondFile in zip(files, reversed(files)):
				self.compare(firstFile, secondFile)
		TraceFormat("###### END_COMPARING ######")

protoChecker = ProtoChecker()
protoChecker.run()