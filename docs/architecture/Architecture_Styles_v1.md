# Architecture: Visual Refactor Strategy

**Version**: 1.0
**Author**: Bob (Architect)
**Status**: APPROVED

## 1. Technical Approach
We will transition from "Inline Styles + Mixed CSS" to a "Single Theme File" architecture.

### 1.1 File Structure Changes
- **Current**: `index.html` (Inline CSS ~800 lines) + `style.css` (Unused or Conflict?).
- **New**:
    - `web/client/css/tech-theme.css`: Main stylesheet containing all "Cyberpunk" styles.
    - `web/client/index.html`: Stripped of `<style>` tag, linking `css/tech-theme.css`.

### 1.2 CSS Strategy
- **CSS Variables (Root)**:
    ```css
    :root {
        --bg-core: #050510;
        --glass-surface: rgba(255, 255, 255, 0.05);
        --neon-cyan: #00f3ff;
        --neon-pink: #bc13fe;
        --font-tech: 'Segoe UI', sans-serif; /* Fallback for now */
        --border-glow: 0 0 10px rgba(0, 243, 255, 0.5);
    }
    ```
- **Reset**: Keep standard reset.
- **Layout**: Flexbox/Grid (preserve existing layout logic where possible, reskinning containers).

### 1.3 Implementation Steps
1.  **Extraction**: Copy inline styles to `temp_old.css` (for reference).
2.  **Creation**: Create `tech-theme.css` with the new Variable definitions.
3.  **Refactoring**: Rewrite component styles (Header, Cards, Buttons, Chat, LED Grid) using the new variables.
4.  **Integration**: Update `index.html` to link the new file and remove the old `<style>`.

## 2. Risks & Mitigations
- **Risk**: Breaking JS Selectors.
    - *Mitigation*: Alex must check `index.html` and `js/*.js` (if any) to ensure class names used in JS (e.g., `.device-card`, `.btn-switch`) are preserved.
- **Risk**: Responsiveness.
    - *Mitigation*: Test with Playwright or manual resize.

## 3. Libraries
- **Chart.js**: Existing library. Will need JS configuration update for colors (Mike/Alex to note this).
- **Fonts**: Consider Google Fonts (e.g. `Orbitron`) if internet access allows. (User has internet).

