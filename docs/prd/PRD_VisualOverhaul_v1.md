# PRD: Visual Overhaul - "Cyberpunk/Tech" Style

**Version**: 1.0
**Author**: Emma (Product Manager)
**Status**: DRAFT

## 1. Background
The current website layout and style (embedded in `index.html` and `style.css`) feels "plastic" and generic (attempting an Apple-like look but failing to impress). The user desires a "Cool", "Tech", and "Sci-Fi" aesthetic.

## 2. Objective
Completely refactor the website's visual style to achieve a "Future Tech" / "Cyberpunk" look.
**Success Metrics**:
- High visual impact ("Wow" factor).
- Modern "Dark Mode" aesthetic.
- Coherent color scheme and typography.
- Mobile responsiveness maintained.

## 3. Design Specifications

### 3.1 Theme & Atmosphere
- **Concept**: Future Control Terminal / Sci-Fi Dashboard.
- **Background**: Deep space dark (`#050510`) or radial gradient dark.
- **Texture**: Grid lines (subtle), Hexagon patterns (optional background).

### 3.2 Color Palette
- **Primary Background**: `#0a0b1e` (Deep Blue/Black)
- **Glass Panel**: `rgba(20, 20, 35, 0.7)` with `backdrop-filter: blur(12px)`
- **Accent 1 (Neon Cyan)**: `#00f3ff` (Buttons, Links, Active States)
- **Accent 2 (Neon Pink/Purple)**: `#bc13fe` (Highlights, Alerts)
- **Text**: `#e0e0e0` (Primary), `#a0a0b0` (Secondary)
- **Borders**: Thin, semi-transparent white/cyan (`rgba(0, 243, 255, 0.3)`)

### 3.3 Typography
- **Headings**: `Orbitron`, `Rajdhani`, or standard `sans-serif` with wide spacing (uppercase).
- **Body**: `JetBrains Mono`, `Fira Code`, or `Roboto` (clean tech look).

### 3.4 Key Components
| Component | Current Style | New "Tech" Style |
| :--- | :--- | :--- |
| **Container** | White shadow card | Glassmorphism panel, glowing border |
| **Buttons** | Rounded generic pill | Sharp edges or chamfered corners, neon glow on hover |
| **Inputs** | Standard border | Underline only or dark field with glowing focus |
| **Header** | Gradient text | "Hologramatic" text effect, tech scanlines |
| **Charts** | Standard Chart.js | Re-style Chart.js colors to Neon (Cyan lines, dark bg) |

## 4. Functional Requirements
- **Refactoring**: Move inline CSS from `index.html` to `web/client/css/tech-theme.css`.
- **Structure**: Update HTML classes to match BEM or utility needs for the new theme.
- **Responsiveness**: Ensure "Tech" layout stacks correctly on mobile.

## 5. Scope
- **In Scope**: `web/client/index.html`, `web/client/css/*`.
- **Out of Scope**: Backend logic (`web/server/*`), ESP32 firmware.
