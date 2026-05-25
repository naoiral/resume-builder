#!/usr/bin/env python3
"""
Build font subset for ResumeBuilder
Creates a minimal TTF containing only the characters needed for Chinese resumes.

Usage:
    python build_font_subset.py [path/to/NotoSansSC-Regular.ttf]

Requirements:
    pip install fonttools brotli

Output:
    NotoSansSC-Regular-subset.ttf (~800KB-1.2MB)
"""

import sys
import os

# GB2312 Level 1 characters (3755 most common simplified Chinese characters)
# This is a subset - in production, the full list would be included
GB2312_SAMPLE = """
的一是不了人我在有他这中大来上个国和地到说时要就出会也你对生能而子那得于着下自之年过发后作里用道行所然家种事成方多经么去法学如都同现当没动面起看定天分还进好小部其些主样理心她本前开但因只从想实日军者意无力它与长把机十民第公此已工使情明性知全三又关点正业外将两高间由问很最重并物手应战向头文体政美相见被利什二等产或新己制身果加西斯月话合回特代内信表化老给世位次度门任常先海通教儿原东声提立及比员解水名真论处走义各入几口认条平系气题活尔更别打女变四神总何电数安少报才结反受目太量再感建务做接必场件计管期市直德资命山金指克许统区保至队形社便空决治展花术飞交受近运笑根干取联步规每即规根观清术更达风集布石容需候厂万确整列温装随片古远收共击造六感类备号始维划技织食格议示层企规głoś
ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789
，。！？、；：""''（）【】《》—…·
,.:;!?()-[]{}'"@#$%^&*+=/<>~`|
"""

def build_subset(font_path, output_path=None):
    try:
        from fontTools.ttLib import TTFont
        from fontTools.subset import Subsetter, Options
    except ImportError:
        print("Error: fonttools not installed. Run: pip install fonttools brotli")
        return False

    if not os.path.exists(font_path):
        print(f"Error: Font file not found: {font_path}")
        print("Please download Noto Sans SC from: https://fonts.google.com/noto/specimen/Noto+Sans+SC")
        return False

    if output_path is None:
        output_path = os.path.join(os.path.dirname(__file__), "NotoSansSC-Regular-subset.ttf")

    print(f"Loading font: {font_path}")
    font = TTFont(font_path)

    # Build character set
    chars = set()
    for ch in GB2312_SAMPLE:
        if ch.strip():
            chars.add(ch)

    print(f"Character set: {len(chars)} characters")

    # Configure subsetter
    options = Options()
    options.flavor = None
    options.hinting = False  # Remove hinting to save space
    options.desubroutinize = True
    options.layout_features = ['*']

    subsetter = Subsetter(options=options)
    subsetter.populate(text=''.join(chars))

    print("Subsetting font...")
    subsetter.subset(font)

    print(f"Saving subset to: {output_path}")
    font.save(output_path, reorderTables=False)

    size_kb = os.path.getsize(output_path) / 1024
    print(f"Done! Subset size: {size_kb:.1f} KB")
    return True

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python build_font_subset.py <path-to-NotoSansSC-Regular.ttf>")
        print("\nDownload Noto Sans SC from:")
        print("https://fonts.google.com/noto/specimen/Noto+Sans+SC")
        sys.exit(1)

    font_path = sys.argv[1]
    build_subset(font_path)
