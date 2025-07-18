# 🖌️ DefectDetect-Application

**A lightweight tool for annotation and patch generation**, originally developed for defect marking on leather surfaces, but easily adaptable for general image annotation needs.

> 📌 *This work was partially supported by the Federal Ministry of Science and Education of Bosnia and Herzegovina under the project "Automated Visual Inspection Based on Deep Neural Networks" in 2023/2024, at Faculty of Electrical Engineering, University of Sarajevo.*

---

## 🔎 Overview

**DefectDetect** enables manual image annotation via freehand drawing, the creation of smaller image segments called **patches**, and flexible export of annotation data in structured JSON format.

Key features:
- 🧩 Patch creation & export with **defect severity ratings (0–2)**
- 📁 Export in both **individual** and **collective** JSON files
- ✅ No installation required — standalone executable

---

## 📁 Project Structure

```
DefectDetect-Application/
├── data/                 # Input test images
├── notebooks/            # Jupyter analysis/evaluation
├── qt_runtime/           # Application source code
├── README.md             # Project description
└── .gitignore
```

---
## 📥 **Download**

🪟 **For Windows App:**
The application does **not require installation** or dependency setup.

1. Go to the [Releases](https://github.com/arapov1c/DefectDetect-Application/releases) section.
2. Download the latest release `.zip` package.
3. Unzip and run the executable file directly.

✅ That’s it — no Python, no installation, just run and annotate!

🐧 **For Linux App**

1. Go to the [Releases](https://github.com/arapov1c/DefectDetect-Application/releases) section.
2. Download the latest '.zip' package for Linux where you can find `DefectDetect-x86_64.AppImage` file (a single-file Linux executable).
3. Make the file executable and run it:

   ```bash
   chmod +x DefectDetect-x86_64.AppImage
   ./DefectDetect-x86_64.AppImage
   
---

## 📖 How to Use

For detailed instructions:

- 🪟 [Windows usage guide](instructions/README_windows.md)
- 🐧 [Linux usage guide](instructions/README_linux.md)

---

## 📦 Export Options

- **Patch-based Export**: Create and save patches as separate images.
- **Ratings**: Assign 0 (none), 1 (partial), or 2 (full) for defect presence in each patch.
- **JSON Files**:
  - **Individual**: One JSON file per patch
  - **Collective**: One JSON file containing all patches, masks, ratings, etc.

---

## 📄 JSON Structure

Each collective JSON file includes:
- `annotations`: patch ID, ratings, mask links
- `classes`: list of defect classes
- `masks`: masks per patch
- `ratings`: meaning of each rating
- `org_patches`: position & size of patches

> For code examples on parsing and using the JSON files in Python (e.g. in Colab), see `notebooks/DefectDetect_JSON_Examples.ipynb`.

---

## 🧠 Improvements Planned

- [ ] Support for rectangle/circle annotations
- [ ] GUI enhancements & internationalization

---

## 📚 Citation

If you use this tool in your research or publications, please cite:

> **Arapović, L. (2024).** _DefectDetect: Application for Defect Annotation on Leather Surfaces_. University of Sarajevo – Faculty of Electrical Engineering. (to be changed)

---

## 📬 Contact

Developed by: **Lejla Arapović**  
Email: `larapovic1@etf.unsa.ba`  
Faculty of Electrical Engineering, University of Sarajevo
