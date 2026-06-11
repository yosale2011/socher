# ארכיטקטורה סופית: "סוחר הים" 32-ביט — ליבה מילולית, מעטפת מודרנית, הוכחה מתמשכת

מסמך זה הוא הסינתזה הסופית של ארבע ההצעות. הבסיס הוא **Design 1 ("הדה-קומפילציה היא המנוע")** — הזוכה בשני שופטים (אותנטיות 9, ישימוּת 8) — עם השתלות מפורשות: מנגנון ה-Oracle הפנימי, פורמט ה-Replay ותיוג קריאות ה-RNG מ-Design 4; טבלת המחרוזות הדו-עמודתית, מסך ההגדרות המחולל ובדיקת השחיתות של cp862 מ-Design 2; ועקרון ה-fallback פר-מסך והדגלים המתועדים לבאגים מ-Design 3.

---

## חזון

הקוד המפוענח `CODE.PAS` + `GLOBALS.PAS` נשאר **המימוש היחיד** של לוגיקת המשחק — לא משכתבים אותו, אלא מקמפלים אותו (אחרי טרנספילציה מכנית וקטנה) תחת Free Pascal עם מעטפת SDL2 מודרנית. הנאמנות למקור איננה הצהרה אלא הוכחה רצה: שער digest מול `socher1/K.com.orig`, ו-traces דטרמיניסטיים שמושווים byte-for-byte מול הבינארי המקורי שרץ ב-DOSBox. כל שדרוג — הגדרות, מכניקות חדשות, גרפיקה משופרת — חי **מחוץ** לליבה, מאחורי עוגני Hook ומתגים, כך ש"מצב קלאסי" נשאר קלאסי באופן שניתן לאמת במכונה. התוצאה ניתנת לשילוח בשלבים על-ידי מפתח יחיד: משחק playable מתקבל מוקדם, וההוכחות מתהדקות בהדרגה.

---

## עקרונות מנחים

1. **הקוד המקורי הוא המנוע.** אין reimplementation של לוגיקה. כל שינוי ידני ב-`CODE.PAS`/`GLOBALS.PAS` חייב לשמר התאמת digest של `build/K.COM` מול `socher1/K.com.orig` (השער הקיים ב-`build/validate.py`).
2. **טרנספילציה מכנית, לא חכמה.** `port/tools/transpile.py` הוא משכתב token-level דטרמיניסטי עם טבלת כללים קטנה (תקרה קשיחה: ~12 כללים). הפלט (`port/gen/*.inc`) מחויב (committed) ל-git כדי שכל diff ייסקר; הרצה חוזרת חייבת להיות אידמפוטנטית.
3. **כל מודרניזציה — במעטפת.** SDL2, הגדרות, Hooks, replays, סאונד — הכול ביחידות FPC סביב הליבה. הליבה לא יודעת שהם קיימים.
4. **דטרמיניזם כחוזה.** המשחק הוא פונקציה: `(seed, settings-hash, key-log) → (state-trace, frame-CRCs, score)`. ה-RNG של TP3 משוחזר bit-exact, כל קריאת Random מתויגת באתר הקריאה (site tag), ו-Delay מתקדם על שעון סימולציה.
5. **Oracle כבדיקת אמת, לא כאוטומציה שבירה.** הבינארי המקורי, מקומפל עם shim אינסטרומנטציה (SEED.TXT / KEYS.TXT / TRACE.LOG), הופך את DOSBox-X ל"מריץ batch טיפש" — בלי AUTOTYPE שביר (השתלה מ-Design 4, שהשופטים דרשו פה-אחד).
6. **באגים מקוריים הם פיצ'רים.** כל quirk מתועד הוא דגל בשם מפורש עם ציטוט שורת-מקור, דולק כברירת-מחדל ב-ClassicMode, וערכיו נגזרים מראיות (לוגים מה-Oracle) ולא מהנחות.
7. **Playable לפני Perfect.** אבן-הדרך של פריטיות מלאה (שבוע שלם) ניתנת לדחייה; משחק שלם וויזואלית-נאמן זמין כבר אחרי headless-first-light + SDL.
8. **עברית ויזואלית cp862 היא נכס בינארי.** מחרוזות בסדר ויזואלי לעולם לא עוברות bidi; הן מוגנות ב-checksum lint ב-CI מפני עורכים מודרניים.

---

## ארכיטקטורה

```
                 +--------------------------------------------+
                 |  CANONICAL CORE (קפוא, cp862, TP3)          |
                 |  CODE.PAS + GLOBALS.PAS                    |
                 +-----+----------------------+---------------+
                       |                      |
        build/ (TPCLI+DOSBox)        port/tools/transpile.py
        validate.py: digest ==       (טבלת כללים + עוגני HOOK)
        socher1/K.com.orig                  |
                       |                    v
                +------+------+      port/gen/code.inc
                |             |      port/gen/globals.inc  (committed)
                v             v             |
          build/K.COM    oracle/            v
          (בינארי תואם)  K-TRACE.COM   +----+------------------------------+
                         shim:         | port/src/socher.lpr  {$MODE TP}   |
                         SEED.TXT      |                                   |
                         KEYS.TXT      |  Tp3Compat.pas (LCG, Int16, Real6)|
                         TRACE.LOG     |  Trace.pas / Replay.pas (.shr)    |
                              |        |  Settings.pas + Hooks.pas         |
                              |        +----+--------------+---------------+
                              |             |              |
                              |     PlatformIntf.pas (חתימות GRAPH.P + RTL)
                              |             |              |
                              |      +------+-----+  +-----+-----------+
                              |      | PlatformSDL|  | PlatformHeadless|
                              |      | (משחק חי)  |  | (CI, מהירות-אור)|
                              |      +------------+  +-----+-----------+
                              |                            |
                              +------------+---------------+
                                           v
                          tests/parity_diff.py + port/golden/*.shr
                          (diff של traces + frame-CRC; שער ב-check_port.py)
```

### רכיבים

- **ליבה קנונית — `CODE.PAS`, `GLOBALS.PAS` (שורש הריפו):** TP3 טהור, בתים cp862, משוחזר להתאמת digest מול `socher1/K.com.orig`. מסך ההגדרות הניסיוני (POC, `CODE.PAS:2595-2630`, `GLOBALS.PAS:131-135`) **יוצא** מהליבה ועובר לשכבת ההזרקה.
- **שער נאמנות — `build/`:** הצינור הקיים (TPCLI.COM ב-DOSBox, `build/validate.py`) ללא שינוי; ירוק = הוכחה ש"הקוד המקורי הוא המנוע". ה-baseline המוכרז: `socher1/K.com.orig`.
- **טרנספיילר — `port/tools/transpile.py`:** כללי ההחלפה: `Read(KBD,Ch)→PortReadKbd`, `KeyPressed→PortKeyPressed`, `Sound/NoSound/Delay→PortSound/PortNoSound/PortDelay`, `Randomize/Random→Tp3Randomize/Tp3Random` (עם הזרקת site-tag אוטומטית לפי שם הפרוצדורה המקיפה — השתלה מ-Designs 3/4), והזרקת עוגני Hook בשמות (`{*HOOK:AfterDayStartEvent*}` → קריאה ל-`Hooks.AfterDayStartEvent`). כל כלל שמשנה סמנטיקה חייב בדיקת פריטיות צמודה.
- **Oracle מאונסטרומנט — `oracle/`** (השתלה מ-Design 4): build TP3 נפרד של הדה-קומפילציה + יחידת shim: seed נקרא מ-SEED.TXT, מקשים מוזנים מ-KEYS.TXT דרך החלפת `Read(KBD)`, וכל קריאת Random + snapshot מצב בגבולות-יום נכתבים ל-TRACE.LOG באותו פורמט JSONL של הפורט. `oracle/run_oracle.py` מרכיב, מריץ DOSBox-X headless וקוצר את הלוג. **חוק הפרדה (מ-Design 2):** הבינארי המאונסטרומנט לעולם אינו מועמד לשער ה-digest — האינסטרומנטציה משנה גודל קוד ועלולה להפריע ל-quirk של זבל-מחסנית.
- **שכבת פלטפורמה — `port/src/PlatformIntf.pas`** (אבולוציה של `port/src/Platform.pas` הקיים): חתימות GRAPH.P מדויקות (GraphColorMode, Palette, ColorTable, GetPic, PutPic...) + shims ל-RTL. שני backends: **`PlatformSDL.pas`** (חלון 320x200 indexed בקנה-מידה שלם, גל מרובע ל-Sound(700), מיפוי מקשים ל-#72/#80/#75/#77/#68/#27/#13, מקש F12 להקלטת סשן ל-.shr) ו-**`PlatformHeadless.pas`** (framebuffer בזיכרון, מקשים מקובץ, Delay על שעון וירטואלי — שבוע שלם במילישניות). מתחת לשניהם: `port/src/Picture.pas` (אחרי תיקון המשתנה `Packed`) ו-`TextGrid.pas` הקיימים.
- **תאימות TP3 — `port/src/Tp3Compat.pas`:** LCG של TP3.01A (משוחזר בדיסאסמבלי של ה-RTL ב-K.COM או בהתאמה אמפירית), Integer 16-ביט עם wraparound `{$R-}{$Q-}`, ו-Tp3Real (המרת Real 6-בתים ↔ double) לתאימות בינארית של `WINNERS.WIN` (string[20] + Real).
- **Trace + Replay — `port/src/Trace.pas`, `port/src/Replay.pas`:** trace JSONL — לכל Random: `{bound, result, site}`; checkpoints של מצב בתחילת/סוף יום ואחרי כל אירוע; **CRC32 של ה-framebuffer בכל WaitForKey** (מ-Design 4). פורמט replay `.shr`: גרסה + seed + מזהה rule-pack + hash הגדרות + לוג מקשים בזמן-סימולציה. **קובץ replay הוא בו-זמנית שמירה, fixture רגרסיה, והוכחת שיא** — שיא שמוגש הוא ה-replay שלו, ניתן לאימות בהרצה headless.
- **מעטפת שדרוגים — `port/src/Settings.pas` + `port/src/Hooks.pas`:** כל מתג מוצהר עם מטא-דטה `(label, default, RngSafe)`; **מסך ההגדרות מחולל מההצהרות** ומציג אזהרה כשמתג מפצל את זרם ה-RNG הקלאסי (מ-Design 2). שלושת מתגי ה-POC (AllowCrewStrike, AllowClosedShipyard, FixedGuardShipPrice) נודדים לכאן. ClassicMode כופה ברירות-מחדל **וחומש באסרציית-ריצה** שאף Hook לא צרך הגרלה.
- **Quirks — חלק מ-`Hooks.pas` (או `port/src/Quirks.pas`):** דגלים בשמות מפורשים עם ציטוטים: `QuirkClosedBankOverwrite` (CODE.PAS:2464 — ההגרלה נצרכת, התוצאה נדרסת — שורד מילולית בליבה), `QuirkUninitIsShipDamaged` (CODE.PAS:1833 — אמולציה שערכיה נגזרים מלוג ה-Oracle בכל מופע, לא מההנחה 182/237/0), `QuirkInputRealCounterReuse` (CODE.PAS:330), `QuirkCountryNameFallthrough` (CODE.PAS:244).
- **טבלת מחרוזות — `port/data/strings.tsv`** (מ-Designs 2/3): עמודה A — בתי cp862 בסדר ויזואלי (escaped + checksum), עמודה B — UTF-8 בסדר לוגי. ClassicMode מצייר רק את עמודה A גליף-אחר-גליף מ-FONTHE8.COM; עמודה B מכינה תרגומים ורנדור RTL עתידי בלי לגעת בליבה.
- **כלי Python קיימים — `port/tools/`:** `decode_pic.py`, `render_scene.py`, `text_layer.py` נשארים **שכבת-אמת לנכסים**: הפלט שלהם הוא הרפרנס הפיקסלי ש-`Picture.pas` חייב לשחזר, בשער `check_port.py`.

### הכרעות בקונפליקטים

| קונפליקט | הכרעה | נימוק |
|---|---|---|
| FPC (1/3/4) מול Python engine (2) | **FPC.** הכלים ב-Python נשארים כשכבת-אמת לנכסים ול-tooling בלבד | Design 2 זורק את יחידות ה-FPC הקיימות ומממש לוגיקה פעמיים; שופט הישימות נתן לו 4 |
| ליבה מילולית (1) מול transplant ידני (4) מול coroutine (3) מול data-DSL (2) | **מילולית-מטורנספלת.** | "לא יכולה לסטות by construction"; כל restructuring הוא סיכון reordering של RNG |
| Oracle: AUTOTYPE (1) מול shim פנימי (4) | **shim פנימי (SEED/KEYS/TRACE).** | שלושת השופטים סימנו את ה-AUTOTYPE כסיכון; ה-shim מבטל אותו לגמרי |
| הרחבה: ליבה קפואה (1, ציון 4 בהרחבה) מול חבילות-תוכן (2, ציון 9) | **פשרה מדורגת:** עוגני Hook + מתגים מוצהרים-מטא-דטה עכשיו; אם יקום צורך אמיתי ב-modding קהילתי, שכבת data (אירועים מוגדרי-TOML) תיבנה **מעל Hooks.pas**, לא בתוך הליבה | מפתח יחיד; "fixed mode is a mod" של Design 2 מושג כאן ב-quirk-flags; ה-DSL הוא tarpit מוכר |
| ערכי IsShipDamaged: קבועים (1/3) מול ראיות (4) | **ראיות.** ה-Oracle מתעד את הערך בפועל בכל מופע, על גרסת DOSBox-X + config נעוצים, והאמולציה ניזונה מהלוג | הערכים תלויי-מחסנית ועלולים להשתנות בין מסלולי קוד |
| WASM/web (4) ו-skin משופר (3) | **לא במסלול הקריטי.** התפר של PlatformIntf מאפשר אותם בעתיד; אם יגיע skin משופר — לפי עקרון ה-fallback פר-מסך של Design 3 | scope creep מוכר של מפתח יחיד |

---

## שימור האופי המקורי

- **אותו טקסט רץ:** כל נוסחה וכל סדר קריאות Random (2 ההגרלות של StartWeek כולל ההגרלה ה"מתה" של ClosedBankDay, 9 הקריאות בסדר קבוע ב-GenerateNewPrices, לולאת ה-reroll של DoDayStartEvent, מחיר השומר, הגרלות הפיראטים) מתבצעים כפי שנכתבו — כי זה אותו מקור, מותמר מכנית.
- **שער בינארי חי:** `build/validate.py` ירוק = `build/K.COM` זהה ל-`socher1/K.com.orig`. "נשמר" נבדק במכונה בכל commit שנוגע בליבה.
- **RNG זהה ביט-ביט:** `Tp3Compat.pas` משחזר את ה-LCG של TP3; seed ניתן להזרקה בשני הצדדים; כלי ה-diff משווה **גם tags וגם ערכים**, כדי לתפוס הגרלה שזזה ביחס ל-prompt (החידוד של Design 2).
- **באגים משוחזרים בכוונה:** דריסת ClosedBankDay שורדת מילולית כולל צריכת ההגרלה; IsShipDamaged הלא-מאותחל מקבל אתחול יחיד מוזרק-טרנספיילר שערכיו מגיעים מלוג ה-Oracle, מסומן כ-bug-emulation תחת ClassicMode; שאר ה-quirks נשארים כמות-שהם.
- **נאמנות פיקסלית:** באפרי GetPic של `.SCR/.WIN/.SGN/.LIN` נטענים ללא שינוי; עיגון PutPic בפינה השמאלית-תחתונה; היפוכי ColorTable(3,2,1,0) להבהוב תפריטים; פלטת CGA 1 קבועה; גליפים 8x8 מ-FONTHE8.COM (offset 604) נצבעים בית-אחר-בית כך שעברית ויזואלית cp862 לעולם לא עוברת bidi. בדיקות snapshot פיקסליות מול צילומי DOSBox + frame-CRC רציף בכל WaitForKey.
- **קצב וצליל:** Delay(15) לדקת-הפלגה באנימציה, צפצוף Sound(700)/Delay(50)/NoSound/Delay(80) — עוברים דרך Delay אמיתי ב-SDL (ומוּאצים רק ב-headless).
- **`WINNERS.WIN` תואם-בתים** (Tp3Real + string[20]) — קובץ שיאים אחד נודד בין DOSBox לפורט.

---

## מסלול השדרוגים

כל שדרוג מציית לכלל אחד: **אסור לו לגעת ב-`port/gen/*.inc` או בליבה.** השכבות, מהקרובה לרחוקה:

1. **שכבת פלטפורמה (אפס השפעה על הליבה):** קנה-מידה/יחס-1.2, הילוך-מהיר לאנימציות (סקיילינג PortDelay), עוצמת קול, מקש screenshot, ייצוא שיאים UTF-8 לצד הקובץ המקורי, שמירה-והמשך באמצע שבוע (סריאליזציה של מצב GLOBALS + מונה RNG).
2. **מתגים ב-`Settings.pas`:** כל מתג עם מטא-דטה `RngSafe`; מסך ההגדרות מחולל אוטומטית; ClassicMode נועל הכול ומפעיל אסרציית אי-צריכת-RNG. מתג שמפצל את הזרם — מוצג עם אזהרה.
3. **מכניקות חדשות ב-`Hooks.pas`:** עוגנים בשמות (AfterDayStartEvent, BeforeTravelEvent, AfterDayEnd, OnPricesGenerated). אירועים חדשים, בנק נושא-ריבית, סחורות נוספות — הכול בפרוצדורות hook; ClassicMode מקצר את כולן. כל ריצה לא-קלאסית מוטבעת ב-replay עם hash ההגדרות, כך שגם ריצות מודרניות דטרמיניסטיות ושיתופיות (מ-Design 4).
4. **רנדור עתידי:** כל הציור עובר ב-PlatformIntf, אז backend "Enhanced" (נכסים מצוירים-מחדש, RTL לוגי מעמודה B של strings.tsv) אפשרי — **פר-מסך, עם fallback של upscale קלאסי** (מ-Design 3), כדי שאמנות לא תחסום שום שחרור.
5. **שכבת data עתידית (אופציונלית):** אם modding קהילתי יהפוך ליעד, מנשק TOML של אירועים ימומש כצרכן של Hooks — בלי לפתוח את הליבה.
6. **כלי איזון:** ה-harness ה-headless מאפשר Monte Carlo על אלפי seeds לשבוע מלא — הערכת השפעת כל מכניקה חדשה על התפלגות הניקוד.

---

## אסטרטגיית בדיקות מול המקור

ארבעה שערים, מהזול ליקר, כולם תחת `port/tools/check_port.py` + GitHub Actions:

1. **שער digest:** `build/validate.py` — הליבה עדיין מקומפלת לבינארי הזהה ל-`socher1/K.com.orig`.
2. **שער טרנספיילר:** הרצת `transpile.py` לא משנה את `port/gen/` (אידמפוטנטיות, אפס drift לא-מחויב) + checksum למחרוזות cp862.
3. **שער פריטיות התנהגותית:** לכל `.shr` ב-`port/golden/` — הרצת ה-Oracle (DOSBox-X כ-batch: SEED.TXT+KEYS.TXT→TRACE.LOG) מול הפורט ה-headless באותו seed ולוג מקשים; `tests/parity_diff.py` משווה JSONL שורה-שורה ומדווח את **ההגרלה הסוטה הראשונה לפי site-tag** + דלתת מצב. הקורפוס גדל אורגנית: F12 ב-SDL מקליט כל סשן אמיתי כמועמד-golden (כיסוי ענפים נדירים: שבי-פיראטים +50 קיבולת, אירוע 15 כפוי, טביעה).
4. **שער פיקסלים:** frame-CRC בכל WaitForKey מול צילומי DOSBox-X (גרסה + config נעוצים), ובנפרד — snapshot מלא של כל סוג מסך כ-fixture. בנוסף, `Picture.pas` נבדק תמידית מול פלט `decode_pic.py`/`render_scene.py`.

יחידות נפרדות: בדיקת bit-exact ל-`Tp3Compat` (200+ ערכי Random(N) לכמה seeds ומודולים מול probe ב-DOSBox), property-tests ל-Trunc/חילוק על Tp3Real מול ערכי Oracle, ו-round-trip ל-`WINNERS.WIN` מול הקובץ ב-`socher1/`.

---

## אבני דרך

- **M0 — עיגון מחדש של ה-baseline:** הוצאת מסך ה-POC מ-`CODE.PAS`/`GLOBALS.PAS` לפאץ' בשכבת ההזרקה; `build/validate.py` חוזר לירוק מול `socher1/K.com.orig`; תיקון `Packed` ב-`port/src/Picture.pas`; התקנת FPC ו-`smoke_picture.lpr`/`smoke_platform.lpr` מקומפלים. *אימות:* digest ירוק + PPM זהה לרנדרים הקיימים.
- **M1 — טרנספילציה וקומפילציה יבשה:** `transpile.py` + `port/gen/*.inc` מחויבים; `socher.lpr` מתקמפל מול PlatformIntf מסטאבּים. *אימות:* fpc יוצא 0; הרצה חוזרת — אפס diff.
- **M2 — שחזור RNG של TP3:** `Tp3Compat`; probe DOS עם seed קבוע שמדפיס 200 ערכים. *אימות:* התאמה ביט-ביט לכמה seeds ומודולים. (מוקדם בכוונה — הכול תלוי בזה.)
- **M3 — Oracle מאונסטרומנט:** `oracle/` shim (SEED/KEYS/TRACE) + `run_oracle.py`. *אימות:* שתי הרצות עם אותו seed+keys → TRACE.LOG זהה; הבינארי הלא-מאונסטרומנט עדיין עובר digest.
- **M4 — Headless first light:** PlatformHeadless + Picture/TextGrid; מקשים מתסריט מריצים Init ויום 1; PPM לכל פריים. *אימות:* מסך פתיחה/ראשי זהה-פיקסלית ל-DOSBox; מחירי יום-1 תואמים trace של Oracle.
- **M5 — בנייה playable ב-SDL:** חלון, מקלדת מלאה כולל F10, צפצוף גל-מרובע, Delay בזמן-אמת, F12 מקליט `.shr`. *אימות:* שבוע מלא במשחק ידני; השוואת תחושה צד-לצד מול DOSBox; snapshots של כל מסך כ-fixtures. **(נקודת "יש משחק" — לפני פריטיות מלאה.)**
- **M6 — פריטיות שבוע מלא:** `Trace.pas` בשני הצדדים; `parity_diff.py` על משחק שבועי מלא. *אימות:* diff ריק (אירועים, מחירים, קרבות, ניקוד סופי) ל-5 seeds לפחות ו-3 תסריטי מקשים שונים; סשנים שהוקלטו ב-F12 מקודמים ל-`port/golden/`.
- **M7 — מעטפת שדרוגים:** `Settings.pas` + `Hooks.pas` + מסך הגדרות מחולל + quirks כדגלים מתועדים; ClassicMode עם אסרציית RNG. *אימות:* פריטיות ירוקה ב-ClassicMode; המתגים משנים התנהגות באופן מדיד כשכבויים.
- **M8 — הקשחת CI:** שרשרת אחת: digest → transpile-drift → fpc → בדיקות RNG → פריטיות headless → frame-CRC/snapshots. *אימות:* פקודה אחת עונה "האם זה עדיין המשחק המקורי?"
- **M9 — שדרוגים ראשונים:** קנה-מידה/קצב, שמירה-והמשך (`.shr` כקובץ שמירה), ייצוא שיאים UTF-8, אירוע hook חדש אחד מסומן RngSafe, שיאים מגובי-replay. *אימות:* כל אחד מאחורי מתג; הפריטיות ירוקה כשהכול קלאסי; `WINNERS.WIN` שנכתב בפורט נקרא ב-DOS.

---

## סיכונים והפחתות

- **קבועי ה-LCG של TP3.01A** עלולים להיות קשים לשחזור (שונים מהתיעוד של TP5+). *הפחתה:* M2 ראשון ועצמאי — דיסאסמבלי של רוטינת Random ב-K.COM או התאמת LCG ל-trace עם seed מוזרק; אם יתגלה מסלול Real ב-Random — מרחיבים את Tp3Compat בלבד.
- **Real 6-בתים מול double:** חישובי כסף/ניקוד עתירי Trunc וחילוק (שלל, נזק, מחיר שומר) עלולים לעגל אחרת. *הפחתה:* רוב האופרנדים שלמים ומדויקים בשני הפורמטים; property-tests מול ערכי-ביניים שה-Oracle מתעד; fallback — shim אריתמטי Tp3Real מלא לנוסחאות הבודדות החמות.
- **IsShipDamaged תלוי-מחסנית** ואינו ניתן ל"שימור", רק לאמולציה. *הפחתה:* גרסת DOSBox-X + config נעוצים כ-Oracle קנוני; הערכים נלקחים מהלוג בכל מופע (ראיות, לא הנחה); מתועד כ-bug-emulation.
- **drift של הטרנספיילר** — אם הכללים "מתחכמים", טענת ה"מילולי" נשחקת. *הפחתה:* פלט מחויב ל-git, בדיקת אידמפוטנטיות ב-CI, תקרת כללים קשיחה, וכל כלל משנה-סמנטיקה מצומד לבדיקת פריטיות.
- **האינסטרומנטציה מפריעה ל-Oracle:** כתיבת קבצים ב-shim עלולה לצרוך RNG או להזיז זבל-מחסנית. *הפחתה:* בינארי TRACE נפרד לחלוטין מיעד ה-digest; בדיקת דטרמיניזם ב-M3; המשחק לא קורא שעון פרט ל-Randomize (מוחלף) ו-Delay (קצב בלבד).
- **שחיתות cp862 ויזואלי** בעורכים מודעי-bidi וב-git. *הפחתה:* `.gitattributes` בינארי-למחצה, אחסון escaped ב-`strings.tsv`, checksum lint ב-`check_port.py`.
- **איכות bindings של SDL2 ל-FPC על Windows.** *הפחתה:* כל קריאות SDL כלואות ב-`PlatformSDL.pas`; `PlatformHeadless` מוכיח שהשאר לא תלוי ב-SDL.
- **scope creep של מפתח יחיד:** M6 (פריטיות מלאה) הוא היקר ביותר. *הפחתה מבנית:* M4+M5 כבר נותנים משחק playable ונאמן-ויזואלית; M6 ניתן להשלמה אינקרמנטלית פר-תת-מערכת; WASM, skin משופר ושכבת ה-data הקהילתית מחוץ למסלול הקריטי בכוונה תחילה.
- **תקרת ההרחבה של ליבה קפואה** (החולשה שזוהתה בשיפוט): מכניקות שדורשות שינוי לוגיקה פנימית לא ייכנסו. *הפחתה:* עוגני Hook בכל צומתי ההחלטה המרכזיים; אם עוגן חסר — הוספתו היא כלל-טרנספיילר אחד נטול-סמנטיקה (הערה→קריאה) ולא עריכת ליבה; ובטווח הרחוק — שכבת data מעל Hooks, כשהצורך יוכח.
