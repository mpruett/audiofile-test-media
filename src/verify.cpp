/*
	Copyright (C) 2011, Michael Pruett. All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.

	3. The name of the author may not be used to endorse or promote products
	derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
	IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <audiofile.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include <yaml-cpp/yaml.h>

#include "md5.h"

static const char *kPath = "path";
static const char *kFileFormat = "fileFormat";
static const char *kSampleRate = "sampleRate";
static const char *kChannels = "channels";
static const char *kFrames = "frames";
static const char *kBytes = "bytes";
static const char *kCompression = "compression";
static const char *kCompression_None = "none";
static const char *kCompression_MS_ADPCM = "msadpcm";
static const char *kCompression_IMA_ADPCM = "ima";
static const char *kCompression_ulaw = "ulaw";
static const char *kCompression_alaw = "alaw";
static const char *kCompression_FLAC = "flac";
static const char *kCompression_ALAC = "alac";
static const char *kSampleFormat = "sampleFormat";
static const char *kByteOrder = "byteOrder";
static const char *kByteOrder_Big = "big";
static const char *kByteOrder_Little = "little";
static const char *kSkip = "skip";
static const char *kInvalid = "invalid";
static const char *kMD5Sum = "md5sum";

static const std::string md5sum(const std::string &path)
{
	MD5Context c;
	MD5Init(&c);
	int fd = ::open(path.c_str(), O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "Could not open file %s\n", path.c_str());
		return "";
	}

	size_t bufferSize = 16 * 1024;
	void *buffer = ::operator new(bufferSize);

	while (true)
	{
		ssize_t bytesRead = ::read(fd, buffer, bufferSize);
		if (bytesRead <= 0)
			break;
		MD5Update(&c, buffer, bytesRead);
	}

	::operator delete(buffer);

	MD5Digest digest;
	MD5Final(&digest, &c);

	::close(fd);

	return MD5DigestToBase16(digest);
}

struct TestResults
{
	int entries;
	int successes;
	int failures;
	int skipped;
	int manifestErrors;

	TestResults()
	{
		entries = 0;
		successes = 0;
		failures = 0;
		skipped = 0;
		manifestErrors = 0;
	}

	void accumulate(const TestResults &results)
	{
		entries += results.entries;
		successes += results.successes;
		failures += results.failures;
		skipped += results.skipped;
		manifestErrors += results.manifestErrors;
	}
};

class ManifestEntry
{
public:
	enum Result
	{
		kSuccess,
		kFailure,
		kSkipped,
		kManifestError
	};

	ManifestEntry(const YAML::Node &e) : m_entry(e), m_failures(0)
	{
	}

	Result process()
	{
		if (const YAML::Node *n = m_entry.FindValue(kSkip))
			return kSkipped;

		if (const YAML::Node *n = m_entry.FindValue(kPath))
		{
			n->GetScalar(m_path);
		}
		else
		{
			logerr("no path specified, line %d", n->GetMark().line);
			return kManifestError;
		}

		if (const YAML::Node *n = m_entry.FindValue(kMD5Sum))
		{
			std::string md5 = md5sum(m_path);
			std::string expectedMD5;
			n->GetScalar(expectedMD5);
			if (md5 != expectedMD5)
			{
				logerr("md5 checksum differs from expected value");
				return kFailure;
			}
		}

		AFfilehandle file = afOpenFile(m_path.c_str(), "r", NULL);

		if (const YAML::Node *n = m_entry.FindValue(kInvalid))
		{
			if (!file)
				return kSuccess;
			logerr("opening invalid file did not fail as expected");
			return kFailure;
		}

		if (!file)
		{
			logerr("could not open file");
			return kFailure;
		}

		for (YAML::Iterator i = m_entry.begin(); i != m_entry.end(); ++i)
		{
			std::string key = i.first().to<std::string>();
			std::string value = i.second().to<std::string>();

			if (key == kFileFormat)
			{
				const char *fileFormat =
					(const char *) afQueryPointer(AF_QUERYTYPE_FILEFMT,
						AF_QUERY_LABEL, afGetFileFormat(file, NULL), 0, 0);
				assert(fileFormat);
				expect(key, std::string(fileFormat), value);
			}
			else if (key == kChannels)
			{
				int expectedChannels = atoi(value.c_str());
				expect(key, expectedChannels,
					afGetChannels(file, AF_DEFAULT_TRACK));
			}
			else if (key == kByteOrder)
			{
				int expectedByteOrder;
				if (value == kByteOrder_Big)
					expectedByteOrder = AF_BYTEORDER_BIGENDIAN;
				else if (value == kByteOrder_Little)
					expectedByteOrder = AF_BYTEORDER_LITTLEENDIAN;
				else
				{
					logerr("bad value for byte order: %s, line %d",
						value.c_str(),
						i.second().GetMark().line);
					return kManifestError;
				}

				expect(key, expectedByteOrder,
					afGetByteOrder(file, AF_DEFAULT_TRACK));
			}
			else if (key == kSampleRate)
			{
				double expectedSampleRate = atof(value.c_str());

				expect(key, expectedSampleRate,
					afGetRate(file, AF_DEFAULT_TRACK));
			}
			else if (key == kSampleFormat)
			{
				std::string width = value.substr(1, value.length() - 1);
				char format = value[0];

				int expectedSampleWidth = atoi(width.c_str());
				bool isValidSampleWidth =
					(expectedSampleWidth >= 1 && expectedSampleWidth <= 32) ||
					expectedSampleWidth == 64;
				if (!isValidSampleWidth)
				{
					logerr("bad value for sample format: %s, line %d",
						value.c_str(), i.second().GetMark().line);
					return kManifestError;
				}

				int expectedSampleFormat = -1;
				switch (format)
				{
					case 's':
						expectedSampleFormat = AF_SAMPFMT_TWOSCOMP; break;
					case 'u':
						expectedSampleFormat = AF_SAMPFMT_UNSIGNED; break;
					case 'f':
						if (expectedSampleWidth == 32)
							expectedSampleFormat = AF_SAMPFMT_FLOAT;
						else if (expectedSampleWidth == 64)
							expectedSampleFormat = AF_SAMPFMT_DOUBLE;
						break;
					default:
						logerr("bad value for sample format: %s, line %d",
							value.c_str(), i.second().GetMark().line);
						return kManifestError;
				}

				int sampleFormat, sampleWidth;
				afGetSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);
				expect(key, expectedSampleFormat, sampleFormat);
				expect(key, expectedSampleWidth, sampleWidth);
			}
			else if (key == kCompression)
			{
				int expectedCompression;
				if (value == kCompression_None)
					expectedCompression = AF_COMPRESSION_NONE;
				else if (value == kCompression_IMA_ADPCM)
					expectedCompression = AF_COMPRESSION_IMA;
				else if (value == kCompression_MS_ADPCM)
					expectedCompression = AF_COMPRESSION_MS_ADPCM;
				else if (value == kCompression_ulaw)
					expectedCompression = AF_COMPRESSION_G711_ULAW;
				else if (value == kCompression_alaw)
					expectedCompression = AF_COMPRESSION_G711_ALAW;
				else if (value == kCompression_FLAC)
					expectedCompression = AF_COMPRESSION_FLAC;
				else if (value == kCompression_ALAC)
					expectedCompression = AF_COMPRESSION_ALAC;
				else
				{
					logerr("bad value for compression: %s, line %d",
						value.c_str(), i.second().GetMark().line);
					return kManifestError;
				}

				expect(key, expectedCompression,
					afGetCompression(file, AF_DEFAULT_TRACK));
			}
			else if (key == kFrames)
			{
				AFframecount expectedFrameCount = atoll(value.c_str());
				expect(key, expectedFrameCount,
					afGetFrameCount(file, AF_DEFAULT_TRACK));

				int bufferFrameCount = 1024;
				int channels = afGetChannels(file, AF_DEFAULT_TRACK);
				int maxBytesPerFrame = 8;
				char *buffer = new char[channels * bufferFrameCount * maxBytesPerFrame];
				AFframecount framesRead = 0;
				while (framesRead < expectedFrameCount)
				{
					AFframecount framesToRead = std::min<AFframecount>(bufferFrameCount,
						expectedFrameCount - framesRead);
					AFframecount result = afReadFrames(file, AF_DEFAULT_TRACK,
						buffer, framesToRead);
					if (result != framesToRead)
					{
						m_failures++;
						break;
					}
					framesRead += result;
				}
				delete [] buffer;
			}
			else if (key == kBytes)
			{
				AFfileoffset expectedTrackBytes = atoll(value.c_str());
				expect(key, expectedTrackBytes,
					afGetTrackBytes(file, AF_DEFAULT_TRACK));
			}
		}

		afCloseFile(file);

		return m_failures == 0 ? kSuccess : kFailure;
	}

private:
	const YAML::Node &m_entry;
	std::string m_path;
	int m_failures;

	template <typename T>
	void expect(const std::string &key, T expectedValue, T actualValue)
	{
		if (expectedValue != actualValue)
		{
			std::cerr << "In processing " << m_path <<
				", wrong value for " << key <<
				": expected " << expectedValue <<
				", got " << actualValue << "." << std::endl;
			m_failures++;
		}
	}

	void logerr(const char *format, ...)
	{
		fprintf(stderr, "In processing %s, ", m_path.c_str());
		va_list ap;
		va_start(ap, format);
		vfprintf(stderr, format, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
};

static std::string basename(const std::string &path)
{
	char *p = new char[path.length() + 1];
	strcpy(p, path.c_str());
	std::string result = basename(p);
	delete [] p;
	return result;
}

static std::string dirname(const std::string &path)
{
	char *p = new char[path.length() + 1];
	strcpy(p, path.c_str());
	std::string result = dirname(p);
	delete [] p;
	return result;
}

static std::string getcwd()
{
	char *p = new char[PATH_MAX];
	char *r = getcwd(p, PATH_MAX);
	std::string result;
	if (r)
		result = r;
	delete [] p;
	return result;
}

bool processManifest(std::string manifestPath, TestResults &accumulatedResults)
{
	fprintf(stderr, "manifest %s\n", manifestPath.c_str());
	std::string base = basename(manifestPath);
	std::string dir = dirname(manifestPath);
	std::string cwd(getcwd());
	chdir(dir.c_str());
	std::ifstream fin(base.c_str());
	YAML::Parser parser(fin);

	TestResults testResults;

	YAML::Node doc;
	while (parser.GetNextDocument(doc))
	{
		for (YAML::Iterator it=doc.begin(); it!=doc.end(); ++it)
		{
			if (it->Type() == YAML::NodeType::Map)
			{
				testResults.entries++;
				ManifestEntry manifestEntry(*it);
				ManifestEntry::Result result = manifestEntry.process();
				if (result == ManifestEntry::kSuccess)
					testResults.successes++;
				else if (result == ManifestEntry::kFailure)
					testResults.failures++;
				else if (result == ManifestEntry::kSkipped)
					testResults.skipped++;
				else if (result == ManifestEntry::kManifestError)
					testResults.manifestErrors++;
			}
		}
	}

	chdir(cwd.c_str());

	fprintf(stderr, "entries: %d, successes: %d, failures: %d, skipped: %d, manifest errors: %d\n",
		testResults.entries,
		testResults.successes,
		testResults.failures,
		testResults.skipped,
		testResults.manifestErrors);
	accumulatedResults.accumulate(testResults);

	return 0;
}

static bool isDirectory(const std::string &path)
{
	struct stat st;
	return stat(path.c_str(), &st) == 0 &&
		S_ISDIR(st.st_mode);
}

void walk(std::string path, TestResults &accumulatedResults)
{
	DIR *d = opendir(path.c_str());
	struct dirent e;
	struct dirent *ep;
	while (!readdir_r(d, &e, &ep) && ep)
	{
		std::string filename = e.d_name;
		std::string filepath = path + "/" + filename;
		if (filename == "." || filename == "..")
			continue;
		if (filename == "manifest.yaml")
			processManifest(filepath, accumulatedResults);
		else if (isDirectory(filepath))
			walk(filepath, accumulatedResults);
	}
}

int main(int argc, char **argv)
{
	if (argc > 2)
		fprintf(stderr, "usage: %s path\n", argv[0]);
	std::string path = argc == 2 ? argv[1] : ".";
	TestResults results;
	walk(path, results);
	fprintf(stderr,
		"total: entries: %d, successes: %d, failures: %d, skipped: %d, manifest errors: %d\n",
		results.entries,
		results.successes,
		results.failures,
		results.skipped,
		results.manifestErrors);
	return results.failures == 0 && results.manifestErrors == 0 ?
		EXIT_SUCCESS : EXIT_FAILURE;
}
