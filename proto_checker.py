#! /usr/bin/env python
# -*- coding: utf-8 -*-
__name__ = "ProtoChecker"
__author__ = "VegaS"
__date__ = "2019-11-18"
__version__ = "0.0.3"

import sys
import os
import string
import csv

# Set the base color of command prompt
os.system("color 00")

# Set working directory
os.chdir(os.path.dirname(os.path.realpath(__file__)))

# Auto detection of vnum range
VNUM_RANGE_ITEM_VNUM_START = 110000 # Start item vnum
VNUM_RANGE_ITEM_VNUM_END = 165400 # End item vnum
VNUM_RANGE = 99 # itemVnumStart~itemVnumEnd+99
VNUM_RANGE_DELIMITER = '~' # Separator

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
	BASE_PATH = "resource\\"
	LOG_FILE_NAME = 'syslog.txt'
	
	ITEM_PROTO_FILE_NAME = BASE_PATH + 'item_proto.txt'
	ITEM_NAMES_FILE_NAME = BASE_PATH + 'item_names.txt'
	MOB_PROTO_FILE_NAME = BASE_PATH + 'mob_proto.txt'
	MOB_NAMES_FILE_NAME = BASE_PATH + 'mob_names.txt'

	def __init__(self):
		self.outputFileList = []

		self.itemProtoDict = self.load_file(self.ITEM_PROTO_FILE_NAME)
		self.itemNamesDict = self.load_file(self.ITEM_NAMES_FILE_NAME)

		self.mobProtoDict = self.load_file(self.MOB_PROTO_FILE_NAME)
		self.mobNamesDict = self.load_file(self.MOB_NAMES_FILE_NAME)

	def load_file(self, fileName):
		fileExists = os.path.exists(fileName)
		if fileExists:
			if os.path.getsize(fileName) == 0:
				self.write_log(FILE_EMPTY.format(fileName))
				self.create_syslog()
				sys.exit(0)

		fileData = []
		vnumRangeSet = set()
		if fileExists:
			with open(fileName, 'r') as csv_file:
				csv_reader = csv.reader(csv_file, delimiter='\t')
				next(csv_reader, None)

				for row in csv_reader:
					if row:
						column = row[0]
						if VNUM_RANGE_DELIMITER in column:
							vnumRangeSet.add(tuple(map(int, column.split(VNUM_RANGE_DELIMITER))))

						fileData.append(column)

		fileData.sort()
		return {'name': fileName, 'data': fileData, 'range': vnumRangeSet, 'exists': fileExists}

	def find_duplicate(self, file):
		if not file["exists"]:
			return

		self.write_log(FILE_NAME.format(file["name"]))

		duplicateSet = set([itemVnum for itemVnum in file["data"] if file["data"].count(itemVnum) > 1])
		for itemVnum in duplicateSet:
			self.write_log(FILE_DUPLICATE_LINE.format(itemVnum, len(file["data"]) - file["data"][-1::-1].index(itemVnum) + 1))

		self.write_log(FILE_TOTAL_LINES.format(duplicateSet.__len__()) if duplicateSet else FILE_OK)

	# noinspection PyDefaultArgument
	def compare(self, firstFile, secondFile):
		differenceList = []

		def check_diff(value, data):
			if value not in data:
				differenceList.append(value)

		if not firstFile['exists'] or not secondFile['exists']:
			return

		self.write_log(FILE_NAME.format(secondFile['name']))

		cmpItemProto = secondFile['name'] == self.ITEM_PROTO_FILE_NAME
		cmpItemNames = secondFile['name'] == self.ITEM_NAMES_FILE_NAME
		if cmpItemProto or cmpItemNames:
			for itemVnum in firstFile['data']:
				baseItemVnum = itemVnum

				# Comparing item_names with item_proto
				if cmpItemProto:
					if not itemVnum.isdigit():
						continue

					intItemVnum = int(itemVnum)
					if intItemVnum in range(VNUM_RANGE_ITEM_VNUM_START, VNUM_RANGE_ITEM_VNUM_END + 1):
						itemVnumRange = '{start}{separator}{end}'.format(start=intItemVnum, separator=VNUM_RANGE_DELIMITER, end=intItemVnum + VNUM_RANGE)
						baseItemVnum = itemVnumRange

					check_diff(baseItemVnum, secondFile['data'])

				# Comparing item_proto with item_names
				elif cmpItemNames:
					if VNUM_RANGE_DELIMITER in itemVnum:
						itemVnumStart, itemVnumEnd = itemVnum.split(VNUM_RANGE_DELIMITER)
						baseItemVnum = itemVnumStart
						
					check_diff(baseItemVnum, secondFile['data'])
		else:
			# Comparing mob_proto with mob_names and vice-versa
			differenceList = list(set(firstFile['data']).difference(secondFile['data']))

		if not differenceList:
			self.write_log(FILE_OK)
			return

		for itemVnum in differenceList:
			self.write_log(FILE_COMPARING_LINE.format(itemVnum))

		self.write_log(FILE_TOTAL_LINES.format(differenceList.__len__()))

	def write_log(self, line, lineDelimiter='\n', formatDelimiter='{}'):
		sys.stdout.write(line + lineDelimiter)

		for color in (COLOR_GREEN, COLOR_RED, COLOR_GRAY):
			for item in color.split(formatDelimiter):
				line = line.replace(item, string.whitespace[0])
		self.outputFileList.append(line)

	def create_syslog(self):
		with open(self.LOG_FILE_NAME, 'w') as out:
			out.write('\n'.join(self.outputFileList))

	def run(self):
		self.write_log("###### START_CHECKING_FOR_DUPLICATE ######")
		for file in (self.itemProtoDict, self.itemNamesDict, self.mobProtoDict, self.mobNamesDict):
			self.find_duplicate(file)
		self.write_log("###### END_CHECKING_FOR_DUPLICATE ######\n")

		self.write_log("###### START_COMPARING ######")
		for files in ((self.itemProtoDict, self.itemNamesDict), (self.mobProtoDict, self.mobNamesDict)):
			for firstFile, secondFile in zip(files, reversed(files)):
				self.compare(firstFile, secondFile)

		self.write_log("###### END_COMPARING ######")
		self.create_syslog()

protoChecker = ProtoChecker()
protoChecker.run()