from pathlib import Path
ROOT = Path(__file__).resolve().parent
path = ROOT / 'build/Data/SKSE/Plugins/TrueHUD/TrueHUD_base.ini'
path.parent.mkdir(parents=True, exist_ok=True)
path.write_text('''; Offline bench fixture only. Do not package this file.
[BossRecognition]
Race = Skyrim.esm:0x012E82
NPC = Skyrim.esm:0x800
RemoveNPC = Skyrim.esm:0x800
NPC = Skyrim.esm:0x802
NPCBlacklist = Skyrim.esm:0x801
LocRefType = Skyrim.esm:0x130F7
''', encoding='ascii')
