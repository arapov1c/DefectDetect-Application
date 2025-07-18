DefectDetect v4.2.1 – Linux Version 🐧

📦 About the Application
DefectDetect is a standalone application for manual annotation and patch creation on high-resolution images.
It supports export in multiple formats (JSON, YOLO, Pascal VOC) and allows user-defined class labeling and patch filtering.

This Linux build is delivered as a portable AppImage – no installation required.

🚀 Getting Started

1. Download DefectDetect-x86_64.AppImage from Releases:
   https://github.com/arapov1c/DefectDetect-Application/releases

2. In terminal, give it executable permission:
   chmod +x DefectDetect-x86_64.AppImage
   ./DefectDetect-x86_64.AppImage

3. Load an image (Load Image) or a previously saved session (Load Session).
   - When loading a session, select the entire Results_<ImageName> folder (with Original, Masks, and Annotation)

4. You can also use Load Multiple Images to iterate through a folder of images using Next/Previous buttons.

5. Use the toolbar to annotate with defect markers or eraser.
   ➤ Annotation is performed by holding the left mouse button.

6. To view masks, create/export patches, or save results, click `End Annotation`.

🧠 Important Usage Notes
-------------------------
🔄 After any of the following:
   - Changing patch dimensions
   - Adding/removing annotations

You MUST click **`End Annotation`** again to update masks and patch data.

⚠️ Warning:
If you export patches again to the **same folder** as before, all previously exported data **will be overwritten**.

⚠️ `End Session` will immediately close the application **without saving**.  
If you wish to preserve current annotations, click **`Save Image`** before ending the session.

⚠️ Limitations in Linux Version
- ❌ Zooming is not supported in this build.

💾 Export & Folder Structure

After clicking `End Annotation`, the export window provides:

- **Show Masks**: View binary masks per each class.
- **Create Patches**: Set dimensions and stride (offset) for patch extraction.
- **Export All Patches**: Export every generated patch and its annotation.
- **Export Selected Patches**: Export only patches that match selected class ratings.

📊 Understanding Patch Ratings
------------------------------
Each patch receives a rating (0, 1, or 2) **per class**, based on annotation coverage:

- `0` – Annotation **not present** in the patch
- `1` – Annotation is **partially present** (based on threshold)
- `2` – Annotation is **fully present** (above threshold)

This allows targeted selection of patches for export or model training.

⚙️ Settings Explanation
-----------------------
Accessible from the main window:
- **Marker Thickness**: Sets the width of the annotation tool.
- **Patch Dimensions and Stride**: Define the size and spacing of extracted patches.
- **Tolerance for Rating 1**:  
  This represents the **percentage of a patch's surface** that must be covered by annotation for it to be rated as:
  - `1`: partial presence  
  - `2`: full presence  
  For example, if set to `95`, a patch must have **more than 95%** coverage to be rated as `2`.

📁 Folder Structure
-------------------

When exporting patches:

    Patches__<ImageName>/
    ├── Patches_<Image>_Mask Defect 1/
    ├── Patches_<Image>_Mask Surface/
    ├── Patches_<Image>_Mask Leather/
    ├── Patches_<Image>_Original/
    ├── json_output/
    ├── Pascal_output/
    ├── yolo_output/

When saving current annotation:

    Results__<ImageName>/
    ├── Original/
    ├── Masks/
    ├── Annotation/

👤 Author
Lejla Arapović
Faculty of Electrical Engineering, University of Sarajevo
📧 larapovic1@etf.unsa.ba
