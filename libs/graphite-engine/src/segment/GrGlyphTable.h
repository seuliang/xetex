/*--------------------------------------------------------------------*//*:Ignore this sentence.
Copyright (C) 1999, 2001 SIL International. All rights reserved.

Distributable under the terms of either the Common Public License or the
GNU Lesser General Public License, as specified in the LICENSING.txt file.

File: GrGlyphTable.h
Responsibility: Sharon Correll
Last reviewed: 10Aug99, JT, quick look over

Description:
    The GrGlyphTable and related classes that store the values of glyph attributes.

	For each glyph in a TrueType font, Graphite has a list of glyph attributes (which
	are defined in the GDL script file). Since most of the values will be zero, we save space
	by only storing the non-zero values.

	Eventually it may be possible to have more than one TrueType font compiled into a single
	glyph table, hence the need for multiple sub-tables. For now we assume that there is only
	one font file and only one sub-table.

	Note that BIG in a variable name means that the data is in big-endian format, or is a range
	of bytes that must be interpreted as a run of wide chars or ints in big-endian format.
-------------------------------------------------------------------------------*//*:End Ignore*/

//:Ignore
#ifdef _MSC_VER
#pragma once
#endif
#ifndef GR_GTABLE_INCLUDED
#define GR_GTABLE_INCLUDED
//:End Ignore

namespace gr
{

class Font;

/*----------------------------------------------------------------------------------------------
	A run of non-zero attribute values for a single glyph, where the attribute IDs
	are contiguous.

	NOTE: it is assumed that the format of this class matches the byte order of the data as it
	is read directly from the TTF/ECF file--that is, it is big-endian.
	
	Hungarian: gatrun
----------------------------------------------------------------------------------------------*/

class GrGlyphAttrRun
{
	friend class GrGlyphTable;
	friend class GrGlyphSubTable;
	friend class GrGlyphAttrTable;

	enum {
		kMaxAttrsPerRun = 255
	};

	/*------------------------------------------------------------------------------------------
		Copy the raw memory (starting at the given byte) into the instance. This method must
		reflect the format of the "Glat_entry" as it is stored in the file and slurped into
		memory.
	------------------------------------------------------------------------------------------*/
	void CopyFrom(byte * pbBIGEnt)
	{
		m_bMinAttrID = *pbBIGEnt;
		m_cAttrs = *(pbBIGEnt + 1);
		if (m_cAttrs >= kMaxAttrsPerRun)
		{
			gAssert(false);
			m_cAttrs = kMaxAttrsPerRun;
		}
		#ifdef _DEBUG
		// Probably not strictly necessary to zero the array, but convenient for debugging.
		// Removed from release build for optimization
		memset(m_rgchwBIGValues, 0, isizeof(m_rgchwBIGValues));
		#endif
		memcpy(m_rgchwBIGValues, pbBIGEnt + 2, (m_cAttrs * isizeof(data16)));
	}

	int ByteCount()
	{
		return (2 + (m_cAttrs * isizeof(data16)));
	}

protected:
	byte m_bMinAttrID;	// ID of first attribute in the run
	byte m_cAttrs;		// number of attributes in the run
	data16 m_rgchwBIGValues[kMaxAttrsPerRun];
};

/*----------------------------------------------------------------------------------------------
	Contains runs of attribute values for all the glyphs in the font; corresponds to
	the "Glat" table in the ECF file.

	Hungarian: gatbl
----------------------------------------------------------------------------------------------*/

class GrGlyphAttrTable
{
	friend class GrGlyphTable;
	friend class GrGlyphSubTable;

protected:
	//	Constructor:
	GrGlyphAttrTable()
	{
		m_prgbBIGEntries = NULL;
	}

	//	Destructor:
	~GrGlyphAttrTable()
	{
		delete[] m_prgbBIGEntries;
	}

	//	Initialization:
	void Initialize(int fxdSilfVersion, int cbBufLen)
	{
		m_fxdSilfVersion = fxdSilfVersion;
		m_cbEntryBufLen = cbBufLen;
		m_prgbBIGEntries = new byte[cbBufLen];
		//	Now the instance is ready to have all the glyph attr entries slurped into
		//	the byte array from the file.
	}

	int GlyphAttr16BitValue(int ibMin, int ibLim, byte bAttrID);

protected:
	int			m_fxdSilfVersion;		// version number of the Silf table, which is used
									// to interpret the attribute values

	//	Block of variable-length glyph attribute runs, matching the format of
	//	GrGlyphAttrRun. We don't store instances of that class here because they
	//	are variable length, and it would be too slow to read them individually from the
	//	ECF file. Instead, we set up a single instance in the method that accesses the values;
	//	having this instance will help debugging. Eventually we may want to do without it,
	//	if it would help efficiency.
	int			m_cbEntryBufLen;	// do we really need this?
	byte *		m_prgbBIGEntries;

//:Ignore
#ifdef OLD_TEST_STUFF
public:
	//	For test procedures:
	void SetUpTestData();
	void SetUpLigatureTest();
	void SetUpLigature2Test();
#endif // OLD_TEST_STUFF

//:End Ignore

};

/*----------------------------------------------------------------------------------------------
	One glyph table per font (style) file; corresponds to the "Gloc" table in the ECF.
	Currently there is only considered to be one style per file, so there is only one of
	these. It holds the (non-zero) glyph attribute values for every glyph in the font.

	Hungarian: gstbl

	Review: Eventually we may need to make a subclass that uses 32-values for the offsets,
	or just use a separate array pointer in this class. Which would be preferable?
----------------------------------------------------------------------------------------------*/

class GrGlyphSubTable
{
	friend class GrGlyphTable;

public:
	//	Constructor:
	GrGlyphSubTable() :
		m_pgatbl(NULL),
		m_prgibBIGAttrValues(NULL),
		m_prgibBIGGlyphAttrDebug(NULL),
		m_prgnDefinedComponents(NULL)
	{
	}
	//	Destructor:
	~GrGlyphSubTable()
	{
		delete m_pgatbl;
		delete[] m_prgibBIGAttrValues;
		if (m_fHasDebugStrings)
			delete[] m_prgibBIGGlyphAttrDebug;
		delete[] m_prgnDefinedComponents;
	}

	//	Initialization:
	bool ReadFromFont(GrIStream & gloc_strm, int cGlyphs, 
		GrIStream & glat_strm, long lGlatStart);
	void Initialize(int fxdSilfVersion, data16 chwFlags,
		data16 chwBWAttr, data16 chwJStrAttr, data16 chwJStrHWAttr,
		int cGlyphs, int cAttrs, int cnCompPerLig);

	void CreateEmpty();

	void SetNumberOfComponents(int c)
	{
		m_cComponents = c;
	}

	//	General:
	int NumberOfGlyphAttrs()	{ return m_nAttrIDLim; }	// 0..(m_nAttrIDLim-1)

	int GlyphAttrValue(gid16 chwGlyphID, int nAttrID);

	//	Ligatures:
	int ComponentContainingPoint(gid16 chwGlyphID, int x, int y);
	bool ComponentBoxLogUnits(float xysEmSquare, gid16 chwGlyphID, int icomp,
		int mFontEmUnits, float dysAscent,
		float * pxsLeft, float * pysTop, float * pxsRight, float * pysBottom,
		bool fTopOrigin = true);
protected:
	int CalculateDefinedComponents(gid16 chwGlyphID);
	bool ComponentIsDefined(gid16 chwGlyphID, int nAttrID);
	int ComponentIndexForGlyph(gid16 chwGlyphID, int nCompID);
//	int NthComponentID(gid16 chwGlyphID, int n);
	int ConvertValueForVersion(int nRet, int nAttrID);

public:
	static int ConvertValueForVersion(int nRet, int nAttrID, int nBWAttr, int fxdVersion);

	//	Flags:
	bool LongFormat(data16 chwFlags)	// 32-bit values?
	{
		return ((chwFlags & 0x01) == 0x01);	// bit 0
	}
	bool HasAttrNames(data16 chwFlags)
	{
		return ((chwFlags & 0x02) == 0x02);	// bit 1
	}
	int GlocLookup(data16 chwGlyphId)
	{
		if (m_fGlocShort)
		{
			unsigned short su = ((unsigned short *)m_prgibBIGAttrValues)[chwGlyphId];
			return lsbf(su);
		}
		else
		{	unsigned int u = ((unsigned int *)m_prgibBIGAttrValues)[chwGlyphId];
			return lsbf((int)u);
		}
	}

protected:
	int					m_fxdSilfVersion;	// version number of the Silf table, which is used
											// to interpret the attribute values
	bool				m_fHasDebugStrings;	// are debugging strings loaded into memory?

	int					m_nAttrIDLim;		// number of glyph attributes
	int					m_cComponents;		// number of initial glyph attributes that
											// represent ligature components
	int					m_cnCompPerLig;

	GrGlyphAttrTable *	m_pgatbl;
	byte *				m_prgibBIGAttrValues;		// byte offsets for glyph attr values
	bool				m_fGlocShort;				// flag for Gloc table format
	data16 *			m_prgibBIGGlyphAttrDebug;	// byte offsets for glyph attr debug strings

	data16	m_chwBWAttr;		// breakweight attr ID; needed for converting
								// between versions

	//	Attr IDs for justify.0.stretch and justify.0.stretchHW; these must always be looked
	//	up in tandem.
	data16	m_chwJStrAttr;
	data16	m_chwJStrHWAttr;

	int * m_prgnDefinedComponents;	// for each glyph, cache list of component attributes that
									// are defined

//:Ignore
#ifdef OLD_TEST_STUFF
public:
	//	For test procedures:
	void SetUpTestData();
	void SetUpLigatureTest();
	void SetUpLigature2Test();
#endif // OLD_TEST_STUFF

//:End Ignore
};

/*----------------------------------------------------------------------------------------------
	Holds all the information about glyph attributes.

	Hungarian: gtbl
----------------------------------------------------------------------------------------------*/

class GrGlyphTable
{
	friend class GrGlyphSubTable;

public:
	//	Constructor:
	GrGlyphTable()
	{
		m_cglf = 0;
		m_cComponents = 0;
		m_cgstbl = 0;
		m_vpgstbl.clear();
	}

	//	Destructor:
	~GrGlyphTable()
	{
		for (int i = 0; i < m_cgstbl; ++i)
			delete m_vpgstbl[i];
	}

	bool ReadFromFont(GrIStream & gloc_strm, long lGlocStart, 
		GrIStream & glat_strm, long lGlatStart, 
		data16 chwBWAttr, data16 chwJStrAttr, int cJLevels, int cnCompPerLig, 
		int fxdSilfVersion);

	void CreateEmpty();

	//	Setters:
	void SetNumberOfGlyphs(int c)
	{
		m_cglf = c;
	}
	void SetNumberOfStyles(int c)
	{
		m_cgstbl = c;
		m_vpgstbl.resize(c);
	}
	void SetNumberOfComponents(int c)
	{
		m_cComponents = c;
		for (unsigned int ipgstbl = 0; ipgstbl < m_vpgstbl.size(); ipgstbl++)
			m_vpgstbl[ipgstbl]->SetNumberOfComponents(c);
	}
	void SetSubTable(int i, GrGlyphSubTable * pgstbl)
	{
		if (signed(m_vpgstbl.size()) <= i)
		{
			gAssert(false);
			m_vpgstbl.resize(i+1);
		}
		m_vpgstbl[i] = pgstbl;
		m_vpgstbl[i]->SetNumberOfComponents(m_cComponents);
	}

	//	Getters:
	int NumberOfGlyphs()		{ return m_cglf; }
	int NumberOfStyles()		{ return m_cgstbl; }

	int NumberOfGlyphAttrs()
	{
		//	All of the sub-tables should have the same number of glyph attributes,
		//	so just ask the first.
		return m_vpgstbl[0]->NumberOfGlyphAttrs();
	}

	int	GlyphAttrValue(gid16 chwGlyphID, int nAttrID)
	{
		gAssert(m_cgstbl == 1);
		return m_vpgstbl[0]->GlyphAttrValue(chwGlyphID, nAttrID);
	}
	//	Eventually:
	//int GlyphAttrValue(gid16 chwGlyphID, int nAttrID, int nStyleIndex)
	//{
	//	return m_vpgstbl[nStyleIndex]->GlyphAttrValue(chwGlyphID, nAttrID);
	//}

	int ComponentContainingPoint(gid16 chwGlyphID, int x, int y)
	{
		gAssert(m_cgstbl == 1);
		return m_vpgstbl[0]->ComponentContainingPoint(chwGlyphID, x, y);
	}
	//	Eventually:
	//int ComponentContainingPoint(gid16 chwGlyphID, int nStyleIndex, int x, int y)
	//{
	//	gAssert(m_cgstbl == 1);
	//	return m_vpgstbl[nStyleIndex]->ComponentContainingPoint(chwGlyphID, nAttrID);
	//}

	bool ComponentBoxLogUnits(float xysEmSquare, gid16 chwGlyphID, int icomp,
		int mFontEmUnits, float dysAscent,
		float * pxsLeft, float * pysTop, float * pxsRight, float * pysBottom,
		bool fTopOrigin = true)
	{
		gAssert(m_cgstbl == 1);
		return m_vpgstbl[0]->ComponentBoxLogUnits(xysEmSquare,
			chwGlyphID, icomp, mFontEmUnits, dysAscent,
			pxsLeft, pysTop, pxsRight, pysBottom,
			fTopOrigin);
	}

	int ComponentIndexForGlyph(gid16 chwGlyphID, int nCompID)
	{
		gAssert(m_cgstbl == 1);
		return m_vpgstbl[0]->ComponentIndexForGlyph(chwGlyphID, nCompID);
	}

	bool IsEmpty()
	{
		return (m_cglf == 0);
	}

protected:
	int m_cglf;			// number of glyphs
	int m_cComponents;	// number of defined components
	int m_cgstbl;		// number of sub-tables, corresponding to font files;
						// for now there is only one

	std::vector<GrGlyphSubTable *> m_vpgstbl;

//:Ignore
#ifdef OLD_TEST_STUFF
public:
	//	For test procedures:
	void SetUpTestData();
	void SetUpLigatureTest();
	void SetUpLigature2Test();
#endif // OLD_TEST_STUFF
//:End Ignore

};

} // namespace gr

#endif // !GR_GTABLE_INCLUDED