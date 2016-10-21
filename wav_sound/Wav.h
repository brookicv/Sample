
# ifndef WAV_H
# define WAV_H


#include <cstdint>
#include <fstream>
#include <string>
#include <memory>

using namespace std;

#define FOURCC uint32_t	

#define MAKE_FOURCC(a,b,c,d) \
( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )

template <char ch0, char ch1, char ch2, char ch3> struct MakeFOURCC{ enum { value = (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24) }; };


// Format chunk data field
struct Wave_format{

	uint16_t format_tag;      // WAVE的数据格式，PCM数据该值为1
	uint16_t channels;        // 声道数
	uint32_t sample_per_sec;  // 采样率
	uint32_t bytes_per_sec;   // 码率，channels * sample_per_sec * bits_per_sample / 8
	uint16_t block_align;     // 音频数据块，每次采样处理的数据大小，channels * bits_per_sample / 8
	uint16_t bits_per_sample; // 量化位数，8、16、32等
	uint16_t ex_size;         // 扩展块的大小，附加块的大小

	Wave_format()
	{
		format_tag = 1; // PCM format data
		ex_size = 0; // don't use extesion field

		channels = 0;
		sample_per_sec = 0;
		bytes_per_sec = 0;
		block_align = 0;
		bits_per_sample = 0;
	}

	Wave_format(uint16_t nb_channel, uint32_t sample_rate, uint16_t sample_bits)
		:channels(nb_channel), sample_per_sec(sample_rate), bits_per_sample(sample_bits)
	{
		format_tag = 0x01;                                           // PCM format data
		bytes_per_sec = channels * sample_per_sec * bits_per_sample / 8; // 码率
		block_align = channels * bits_per_sample / 8;
		ex_size = 0;                                               // don't use extension field
	}
};

// The basic chunk of RIFF file format
struct Base_chunk{

	FOURCC fcc;    // FourCC id
	uint32_t cb_size; // 数据域的大小

	Base_chunk(FOURCC fourcc)
		: fcc(fourcc)
	{
		cb_size = 0;
	}
};

/*

数据格式为PCM的WAV文件的基本结构
--------------------------------
| Base_chunk | RIFF	|
---------------------
|	WAVE            |
---------------------
| Base_chunk | fmt  |	Header
---------------------
| Wave_format|      |
---------------------
| Base_chunk | data |
---------------------------------
|    PCM data                   |
---------------------------------
*/

/*

数据格式为PCM的WAV文件头
--------------------------------
| Base_chunk | RIFF	|
---------------------
|	WAVE            |
---------------------
| Base_chunk | fmt  |	Header
---------------------
| Wave_format|      |
---------------------
| Base_chunk | data |
--------------------------------
*/

struct Wave_header{

	shared_ptr<Base_chunk> riff;
	FOURCC wave_fcc;
	shared_ptr<Base_chunk> fmt;
	shared_ptr<Wave_format>  fmt_data;
	shared_ptr<Base_chunk> data;

	Wave_header(uint16_t nb_channel, uint32_t sample_rate, uint16_t sample_bits)
	{
		riff = make_shared<Base_chunk>(MakeFOURCC<'R', 'I', 'F', 'F'>::value);
		fmt = make_shared<Base_chunk>(MakeFOURCC<'f', 'm', 't', ' '>::value);
		fmt->cb_size = 18;

		fmt_data = make_shared<Wave_format>(nb_channel, sample_rate, sample_bits);
		data = make_shared<Base_chunk>(MakeFOURCC<'d', 'a', 't', 'a'>::value);

		wave_fcc = MakeFOURCC<'W', 'A', 'V', 'E'>::value;
	}

	Wave_header()
	{
		riff = nullptr;
		fmt = nullptr;

		fmt_data = nullptr;
		data = nullptr;

		wave_fcc = 0;
	}
};



/*

	Write and read wave file
	example:
	// Write Wav file
	Wave_header header(1, 48000, 16);

	uint32_t length = header.fmt_data->sample_per_sec * 10 * header.fmt_data->bits_per_sample / 8;
	uint8_t *data = new uint8_t[length];

	memset(data, 0x80, length);

	CWaveFile::write("e:\\test1.wav", header, data, length);

	// Read wave file
	CWaveFile wave;
	wave.read("e:\\test1.wav");
	wave.data // PCM data
*/

class  CWaveFile
{

public:

	CWaveFile()
	{
		header = nullptr;
		data = nullptr;
	}

	void static write_header(ofstream &ofs, const Wave_header &header)
	{
		// Write header

		// Write RIFF
		char *chunk = (char*)header.riff.get();
		ofs.write(chunk, sizeof(Base_chunk));

		// Write WAVE fourcc
		ofs.write((char*)&(header.wave_fcc), 4);

		// Write fmt
		chunk = (char*)header.fmt.get();
		ofs.write(chunk, sizeof(Base_chunk));

		// Write fmt_data
		chunk = (char*)header.fmt_data.get();
		ofs.write(chunk, header.fmt->cb_size);

		// Write data
		chunk = (char*)header.data.get();
		ofs.write(chunk, sizeof(Base_chunk));
	}

	// Write wav file
	bool static write(const string& filename, const Wave_header &header, void *data, uint32_t length)
	{
		ofstream ofs(filename, ofstream::binary);
		if (!ofs)
			return false;

		// Calculate size of RIFF chunk data
		header.data->cb_size = ((length + 1) / 2) * 2;
		header.riff->cb_size = 4 + 4 + header.fmt->cb_size + 4 + 4 + header.data->cb_size + 4;

		// Write header

		// Write RIFF
		char *chunk = (char*)header.riff.get();
		ofs.write(chunk, sizeof(Base_chunk));

		// Write WAVE fourcc
		ofs.write((char*)&(header.wave_fcc), 4);

		// Write fmt
		chunk = (char*)header.fmt.get();
		ofs.write(chunk, sizeof(Base_chunk));

		// Write fmt_data
		chunk = (char*)header.fmt_data.get();
		ofs.write(chunk, header.fmt->cb_size);

		// Write data
		chunk = (char*)header.data.get();
		ofs.write(chunk, sizeof(Base_chunk));

		// Write data
		ofs.write((char*)data, length);

		ofs.close();
		return true;
	}

	// Read wav file
	bool read(const string &filename)
	{
		ifstream ifs(filename, ifstream::binary);
		if (!ifs)
			return false;

		header = make_unique<Wave_header>();

		// Read RIFF chunk
		FOURCC fourcc;
		ifs.read((char*)&fourcc, sizeof(FOURCC));

		if (fourcc != MakeFOURCC<'R', 'I', 'F', 'F'>::value) // 判断是不是RIFF
			return false;
		Base_chunk riff_chunk(fourcc);
		ifs.read((char*)&riff_chunk.cb_size, sizeof(uint32_t));

		header->riff = make_shared<Base_chunk>(riff_chunk);

		// Read WAVE FOURCC
		ifs.read((char*)&fourcc, sizeof(FOURCC));
		if (fourcc != MakeFOURCC<'W', 'A', 'V', 'E'>::value)
			return false;
		header->wave_fcc = fourcc;

		// Read format chunk
		ifs.read((char*)&fourcc, sizeof(FOURCC));
		if (fourcc != MakeFOURCC<'f', 'm', 't', ' '>::value)
			return false;

		Base_chunk fmt_chunk(fourcc);
		ifs.read((char*)&fmt_chunk.cb_size, sizeof(uint32_t));

		header->fmt = make_shared<Base_chunk>(fmt_chunk);

		// Read format data
		Wave_format format;
		ifs.read((char*)&format, fmt_chunk.cb_size);

		// Read data chunk
		ifs.read((char*)&fourcc, sizeof(fourcc));
		if (fourcc != MakeFOURCC<'d', 'a', 't', 'a'>::value)
			return false;

		Base_chunk data_chunk(fourcc);
		ifs.read((char*)&data_chunk.cb_size, sizeof(uint32_t));

		header->data = make_shared<Base_chunk>(data_chunk);

		// 读取 PCM data
		data = unique_ptr<uint8_t[]>(new uint8_t[header->data->cb_size]);
		ifs.read((char*)(data.get()), header->data->cb_size);

		ifs.close();

		return true;
	}

public:
	shared_ptr<Wave_header> header;
	unique_ptr<uint8_t[]> data;
};

# endif