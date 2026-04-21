#include <M5Dial.h>

// ─── Tune these ────────────────────────────────────────────────────────
#define SKIP  4      // encoder ticks per scene change (try 3–6)
#define TOTAL 10
// ──────────────────────────────────────────────────────────────────────

M5Canvas canvas(&M5Dial.Display);

int  currentIndex = 0;
long lastEncoder  = 0;
bool needRedraw   = true;

const char* names[TOTAL] = {
  "Sun", "Venus", "Moon", "Mars", "Jupiter",
  "Saturn", "Neptune", "Kepler-452b", "Andromeda", "Pillars"
};

// ─── Colour macro ─────────────────────────────────────────────────────
#define C(r,g,b) canvas.color565(r,g,b)

// ─── Utility: scatter stars ───────────────────────────────────────────
void stars(int count, uint8_t brightness = 200) {
  for (int i = 0; i < count; i++) {
    uint8_t b = brightness - random(60);
    uint8_t sz = random(100) < 15 ? 2 : 1;
    int sx = random(240), sy = random(240);
    if (sz == 2) canvas.fillCircle(sx, sy, 1, C(b, b, b+20));
    else         canvas.drawPixel(sx, sy, C(b, b, b+20));
  }
}

// ─── Utility: draw a circular vignette ring ───────────────────────────
void vignette() {
  for (int r = 119; r >= 115; r--)
    canvas.drawCircle(120, 120, r, C(0, 0, 0));
}

// ─── Scene drawers ────────────────────────────────────────────────────

void drawSun() {
  canvas.fillScreen(C(2, 2, 8));
  stars(30, 160);

  // outer diffuse corona
  for (int r = 90; r >= 70; r--)
    canvas.drawCircle(120, 120, r, C(80, 30, 0));

  // mid corona rays
  for (int a = 0; a < 360; a += 15) {
    float rad = a * DEG_TO_RAD;
    float jitter = random(10);
    for (float t = 68; t < 88 + jitter; t += 1.5f) {
      int rx = 120 + t * cos(rad), ry = 120 + t * sin(rad);
      canvas.drawPixel(rx, ry, C(180, 80, 0));
    }
  }

  // photosphere layers
  for (int r = 65; r >= 10; r -= 5) {
    uint8_t rr = map(r, 10, 65, 255, 200);
    uint8_t gg = map(r, 10, 65, 240, 120);
    uint8_t bb = map(r, 10, 65, 180, 0);
    canvas.fillCircle(120, 120, r, C(rr, gg, bb));
  }

  // surface convection cells
  for (int i = 0; i < 14; i++) {
    float a = random(360) * DEG_TO_RAD;
    float dist = random(30) + 5;
    int cx2 = 120 + dist * cos(a), cy2 = 120 + dist * sin(a);
    canvas.fillCircle(cx2, cy2, random(4)+3, C(255, 180, 20));
    canvas.drawCircle(cx2, cy2, random(4)+4, C(200, 100, 0));
  }

  // hot core
  canvas.fillCircle(120, 120, 18, C(255, 255, 220));
}

void drawVenus() {
  canvas.fillScreen(C(4, 2, 12));
  stars(20, 150);

  // atmosphere glow
  for (int r = 75; r >= 68; r--)
    canvas.drawCircle(120, 120, r, C(100, 70, 10));

  canvas.fillCircle(120, 120, 68, C(180, 148, 60));

  // thick cloud bands
  uint16_t bcolors[] = {
    C(210, 178, 90), C(160, 118, 40), C(230, 200, 120),
    C(150, 110, 35), C(215, 185, 100), C(140, 100, 30),
    C(225, 195, 110)
  };
  for (int i = 0; i < 7; i++) {
    int yy = 120 - 54 + i * 16;
    int hw = (int)sqrt(max(0, 68*68 - (yy-120)*(yy-120)));
    canvas.fillRect(120 - hw, yy, hw*2, 10, bcolors[i]);
  }
  // limb darkening
  for (int r = 68; r >= 62; r--)
    canvas.drawCircle(120, 120, r, C(80, 55, 10));
}

void drawMoon() {
  canvas.fillScreen(C(2, 2, 6));
  stars(70, 210);

  // earthshine glow
  for (int r = 74; r >= 70; r--)
    canvas.drawCircle(120, 120, r, C(30, 30, 40));

  canvas.fillCircle(120, 120, 70, C(155, 152, 162));

  // albedo variation
  canvas.fillCircle(105, 110, 30, C(170, 168, 178));
  canvas.fillCircle(138, 125, 22, C(140, 138, 148));

  // named craters (stylised)
  struct { int x,y,r; } cr[] = {
    {94,98,13},{140,90,9},{112,138,11},{148,142,7},
    {88,148,9},{128,72,6},{100,158,5}
  };
  for (auto& c : cr) {
    canvas.fillCircle(c.x, c.y, c.r,   C(115,112,122));
    canvas.drawCircle(c.x, c.y, c.r,   C(95, 92,102));
    canvas.drawCircle(c.x, c.y, c.r+1, C(175,172,182)); // bright rim
  }

  // terminator shading
  for (int y = 50; y <= 190; y++) {
    int hw = (int)sqrt(max(0, 70*70-(y-120)*(y-120)));
    // subtle right-side shading
    for (int x = 120+hw-8; x <= 120+hw; x++)
      canvas.drawPixel(x, y, C(60,58,68));
  }
}

void drawMars() {
  canvas.fillScreen(C(3, 2, 8));
  stars(50, 190);

  // dust haze
  for (int r = 74; r >= 70; r--)
    canvas.drawCircle(120, 120, r, C(90, 30, 10));

  canvas.fillCircle(120, 120, 70, C(160, 58, 22));

  // albedo regions
  canvas.fillCircle(120, 120, 67, C(185, 78, 32));
  canvas.fillEllipse(100, 105, 28, 20, C(140, 48, 16));
  canvas.fillEllipse(148, 130, 22, 16, C(170, 65, 25));

  // polar ice cap
  canvas.fillEllipse(120, 56, 20, 10, C(225, 230, 240));
  canvas.fillEllipse(120, 59, 14,  6, C(240, 245, 255));

  // Valles Marineris canyon system
  for (int i = 0; i < 3; i++) {
    canvas.drawLine(82, 118+i*3, 168, 124+i*3, C(100, 28, 8));
    canvas.drawLine(82, 119+i*3, 168, 125+i*3, C(80,  20, 5));
  }

  // Olympus Mons hint
  canvas.fillCircle(88, 100, 8, C(200, 88, 40));
  canvas.fillCircle(88, 100, 4, C(220, 100, 50));

  // limb
  for (int r = 70; r >= 66; r--)
    canvas.drawCircle(120, 120, r, C(60, 18, 5));
}

void drawJupiter() {
  canvas.fillScreen(C(2, 2, 8));
  stars(30, 170);

  canvas.fillCircle(120, 120, 72, C(185, 148, 108));

  // atmospheric bands — alternating warm/cold
  struct BandInfo { int y; uint8_t r,g,b; int h; };
  BandInfo bands[] = {
    {60,  190,160,120, 10},
    {70,  140, 88, 48, 12},
    {82,  210,180,140,  8},
    {90,  150, 95, 55, 14},
    {104, 200,168,128, 10},
    {114, 130, 78, 40, 12},
    {126, 205,175,135, 10},
    {136, 145, 90, 52, 12},
    {148, 195,162,122, 10},
    {158, 135, 82, 44, 12},
    {168, 205,178,140,  8},
  };
  for (auto& b : bands) {
    int dy = b.y - 120;
    int hw = (int)sqrt(max(0, 72*72 - dy*dy));
    canvas.fillRect(120-hw, b.y, hw*2, b.h, C(b.r, b.g, b.b));
  }

  // Great Red Spot — layered ellipses
  canvas.fillEllipse(140, 126, 22, 13, C(140, 40, 20));
  canvas.fillEllipse(140, 126, 18, 10, C(175, 60, 35));
  canvas.fillEllipse(140, 126, 12,  7, C(200, 80, 50));
  canvas.fillEllipse(140, 126,  7,  4, C(220,100, 65));

  // limb darkening
  for (int r = 72; r >= 66; r--)
    canvas.drawCircle(120, 120, r, C(40, 24, 10));
}

void drawSaturn() {
  canvas.fillScreen(C(2, 2, 8));
  stars(50, 180);

  // shadow of rings on planet (ellipse behind)
  canvas.fillEllipse(120, 138, 90, 14, C(30, 22, 8));

  // ring system — back half
  for (int r = 108; r >= 75; r -= 3) {
    uint8_t bright = map(r, 75, 108, 100, 160);
    canvas.drawEllipse(120, 120, r, (int)(r*0.22f), C(bright, bright-20, bright-50));
  }

  // planet disc
  canvas.fillCircle(120, 120, 58, C(200, 172, 108));

  // subtle bands
  for (int i = 0; i < 5; i++) {
    int yy = 100 + i*10;
    int dy = yy - 120;
    int hw = (int)sqrt(max(0, 58*58 - dy*dy));
    canvas.fillRect(120-hw, yy, hw*2, 5, C(180, 150, 85));
  }
  canvas.fillCircle(120, 120, 55, C(210, 182, 118));

  // ring system — front half (over planet bottom)
  for (int r = 108; r >= 75; r -= 3) {
    uint8_t bright = map(r, 75, 108, 120, 180);
    // only draw bottom arc
    for (int a = 0; a <= 180; a += 2) {
      float rad = a * DEG_TO_RAD;
      int rx = 120 + r*cos(rad), ry = 120 + (int)(r*0.22f)*sin(rad);
      if (ry > 120) canvas.drawPixel(rx, ry, C(bright, bright-20, bright-50));
    }
  }

  // limb
  for (int r = 58; r >= 53; r--)
    canvas.drawCircle(120, 120, r, C(80, 60, 25));
}

void drawNeptune() {
  canvas.fillScreen(C(1, 1, 6));
  stars(65, 200);

  // glow
  for (int r = 76; r >= 70; r--)
    canvas.drawCircle(120, 120, r, C(10, 20, 100));

  canvas.fillCircle(120, 120, 70, C(22, 48, 170));
  canvas.fillCircle(120, 120, 67, C(30, 60, 200));

  // subtle band structure
  for (int i = 0; i < 4; i++) {
    int yy = 100 + i*12;
    int dy = yy-120;
    int hw = (int)sqrt(max(0, 67*67-dy*dy));
    canvas.fillRect(120-hw, yy, hw*2, 5, C(25, 50, 180));
  }

  // Great Dark Spot
  canvas.fillEllipse(105, 112, 18, 11, C(15, 28, 110));
  canvas.drawEllipse(105, 112, 18, 11, C(40, 80, 220));

  // bright cloud streaks
  canvas.fillRect(72,  128, 70, 3, C(160,190,255));
  canvas.fillRect(75,  134, 60, 2, C(140,170,240));
  canvas.fillRect(88,  118, 45, 2, C(150,180,250));

  // thin ring hint
  canvas.drawEllipse(120, 120, 82, 12, C(50, 60, 140));

  for (int r = 70; r >= 65; r--)
    canvas.drawCircle(120, 120, r, C(8, 18, 80));
}

void drawKepler() {
  canvas.fillScreen(C(1, 2, 8));
  stars(80, 200);

  // host star hint — small warm glow top-right
  canvas.fillCircle(192, 42, 10, C(255, 210, 120));
  canvas.fillCircle(192, 42,  6, C(255, 240, 200));
  for (int r = 14; r <= 22; r += 2)
    canvas.drawCircle(192, 42, r, C(180, 100, 20));

  // atmosphere glow
  for (int r = 76; r >= 70; r--)
    canvas.drawCircle(120, 120, r, C(20, 50, 140));

  // ocean world base
  canvas.fillCircle(120, 120, 70, C(20, 60, 180));
  canvas.fillCircle(120, 120, 67, C(28, 78, 200));

  // continents
  canvas.fillEllipse(106, 104, 22, 18, C(38, 128, 55));
  canvas.fillEllipse(138, 118, 18, 14, C(42, 135, 50));
  canvas.fillEllipse(100, 138, 15, 10, C(35, 120, 48));
  canvas.fillEllipse(130, 96,  12,  8, C(50, 140, 60));

  // cloud systems — multi-layer ellipses
  canvas.fillEllipse(120,  88, 30,  7, C(180, 210, 255));
  canvas.fillEllipse(105, 148, 24,  6, C(180, 210, 255));
  canvas.fillEllipse(148, 128, 20,  5, C(190, 215, 255));

  for (int r = 70; r >= 65; r--)
    canvas.drawCircle(120, 120, r, C(10, 35, 110));
}

void drawAndromeda() {
  canvas.fillScreen(C(1, 1, 5));
  stars(90, 190);

  // outer halo — very faint
  for (int r = 95; r >= 80; r -= 3)
    canvas.drawCircle(120, 120, r, C(20, 18, 35));

  // galaxy disc — tiled ellipses
  for (int r = 75; r >= 10; r -= 5) {
    uint8_t bright = map(r, 10, 75, 255, 30);
    uint8_t blue   = map(r, 10, 75, 240, 50);
    canvas.drawEllipse(120, 120, r, (int)(r*0.38f), C(bright, bright-20, blue));
  }

  // spiral arm dots
  for (int a = 0; a < 720; a += 4) {
    float progress = a / 720.0f;
    float radius = 12 + progress * 68;
    float rad1 = (a + progress*40) * DEG_TO_RAD;
    float rad2 = rad1 + PI;
    if (radius < 74) {
      int x1 = 120 + radius*cos(rad1), y1 = 120 + radius*sin(rad1)*0.38f;
      int x2 = 120 + radius*cos(rad2), y2 = 120 + radius*sin(rad2)*0.38f;
      uint8_t b = map(radius, 12, 74, 255, 80);
      canvas.drawPixel(x1, y1, C(b, b-10, b+20));
      canvas.drawPixel(x2, y2, C(b, b-10, b+20));
    }
  }

  // bright nucleus
  canvas.fillCircle(120, 120,  8, C(255, 245, 210));
  canvas.fillCircle(120, 120,  4, C(255, 255, 240));

  // dust lane
  canvas.fillEllipse(120, 126, 50, 4, C(5, 3, 10));
}

void drawPillars() {
  canvas.fillScreen(C(14, 4, 28));

  // nebula gradient background — horizontal bands
  for (int y = 0; y < 240; y++) {
    uint8_t rr = 20 + (y * 30) / 240;
    uint8_t gg = 4  + (y * 10) / 240;
    uint8_t bb = 40 + (y * 60) / 240;
    canvas.drawLine(0, y, 239, y, C(rr, gg, bb));
  }

  // stars in nebula
  stars(40, 220);

  // nebula wisps — scattered ellipses
  for (int i = 0; i < 12; i++) {
    int wx = random(240), wy = random(240);
    canvas.fillEllipse(wx, wy, random(20)+8, random(6)+2, C(60+random(60), 10+random(20), 80+random(40)));
  }

  // three pillars — with texture
  struct { int x, w, top; uint8_t r,g,b; } pillars[] = {
    {72,  24, 50,  28, 12, 6},
    {118, 19, 72,  24, 10, 5},
    {158, 14, 88,  20,  8, 4},
  };
  for (auto& p : pillars) {
    // main body
    canvas.fillRect(p.x - p.w/2, p.top, p.w, 240-p.top, C(p.r, p.g, p.b));
    // left dust edge
    canvas.fillRect(p.x - p.w/2,     p.top, 4, 240-p.top, C(p.r*3, p.g*3, p.b*2));
    // right dust edge
    canvas.fillRect(p.x + p.w/2 - 4, p.top, 4, 240-p.top, C(p.r*3, p.g*3, p.b*2));

    // tip — ionisation glow
    canvas.fillEllipse(p.x, p.top + 4, p.w/2+6, 6, C(255, 160, 40));
    canvas.fillEllipse(p.x, p.top + 4, p.w/2+2, 4, C(255, 200, 80));
    canvas.fillEllipse(p.x, p.top + 4, p.w/2-2, 2, C(255, 240,140));

    // EGG-like protrusions on pillar sides
    for (int e = 0; e < 3; e++) {
      int ey = p.top + 20 + e*30 + random(10);
      canvas.fillCircle(p.x - p.w/2 - 3, ey, 4, C(p.r*2, p.g*2, p.b));
      canvas.fillCircle(p.x + p.w/2 + 3, ey, 3, C(p.r*2, p.g*2, p.b));
    }
  }
}

// ─── Dispatcher ───────────────────────────────────────────────────────
void drawBody(int idx) {
  switch (idx) {
    case 0: drawSun();       break;
    case 1: drawVenus();     break;
    case 2: drawMoon();      break;
    case 3: drawMars();      break;
    case 4: drawJupiter();   break;
    case 5: drawSaturn();    break;
    case 6: drawNeptune();   break;
    case 7: drawKepler();    break;
    case 8: drawAndromeda(); break;
    case 9: drawPillars();   break;
  }
}

// ─── UI overlay ───────────────────────────────────────────────────────
void drawOverlay(int idx) {
  // dim bottom strip for label readability
  for (int y = 192; y < 220; y++) {
    int hw = (int)sqrt(max(0, 120*120 - (y-120)*(y-120)));
    canvas.fillRect(120-hw, y, hw*2, 1, C(0,0,0));
    // re-alpha trick: overwrite with semi-transparent layer approximation
  }

  // name label — centred
  canvas.setTextDatum(middle_center);
  canvas.setTextSize(2);
  canvas.setTextColor(C(255, 255, 255));
  canvas.drawString(names[idx], 120, 206);

  // index dots — bottom arc
  int dotCount = TOTAL;
  for (int i = 0; i < dotCount; i++) {
    float angle = (-90 + (i - dotCount/2.0f + 0.5f) * 14) * DEG_TO_RAD;
    int dx = 120 + 100 * cos(angle);
    int dy = 120 + 100 * sin(angle);
    if (i == idx)
      canvas.fillCircle(dx, dy, 4, C(255, 220, 80));
    else
      canvas.fillCircle(dx, dy, 2, C(80, 80, 80));
  }

  vignette();
}

// ─── Setup ────────────────────────────────────────────────────────────
void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  Serial.begin(115200);
  M5Dial.Display.setRotation(0);
  canvas.createSprite(240, 240);
  M5Dial.Encoder.readAndReset();
  //Serial.println("Space Gallery — turn the knob");
}

// ─── Loop ─────────────────────────────────────────────────────────────
void loop() {
  M5Dial.update();

  long enc   = M5Dial.Encoder.read();
  long delta = enc - lastEncoder;

  if (abs(delta) >= SKIP) {
    lastEncoder   = enc;
    currentIndex += (delta > 0) ? 1 : -1;

    // ── wrap both directions ──────────────────────────────────────
    if (currentIndex >= TOTAL) currentIndex = 0;
    if (currentIndex < 0)      currentIndex = TOTAL - 1;

    needRedraw = true;
    //Serial.print("Signal: ");
    //Serial.print(currentIndex + 1);
    //Serial.print(" / ");
    //Serial.print(TOTAL);
    //Serial.print("  →  ");
    //Serial.println(names[currentIndex]);
    Serial.println(currentIndex+1);
  }

  if (M5Dial.BtnA.wasPressed()) {
    canvas.fillSprite(TFT_WHITE);
    canvas.pushSprite(0, 0);
    delay(60);
    needRedraw = true;
    //Serial.print("SELECTED: ");
    //Serial.println(names[currentIndex]);
    
  }

  if (needRedraw) {
    drawBody(currentIndex);
    drawOverlay(currentIndex);
    canvas.pushSprite(0, 0);
    needRedraw = false;
  }
}