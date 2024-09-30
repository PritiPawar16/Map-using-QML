Route Plotter & Distance Calculator

A dynamic application built with QML for plotting routes on interactive maps using GPS data. It processes latitude and longitude coordinates from GPS files, calculates distances between points, and updates data in a user-friendly table. Perfect for navigation, tracking, and data analysis.


📚 Table of Contents

Features

Demo

Prerequisites

Installation

Configuration

Usage

Project Structure

Contributing

License

Contact

🚀 Features

Interactive Map Plotting: Visualize routes on an embedded map using QML
.
GPS Data Integration: Import and parse GPS files containing latitude and longitude data.

Distance Calculation: Compute distances between consecutive points on the route.

Dynamic Data Table: View and update GPS data and calculated distances in a structured table.

Real-Time Updates: Seamlessly update map and table as new data is imported or modified.

User-Friendly Interface: Intuitive design for easy navigation and operation.

🎥 Demo

Main Interface showcasing the map and data table.


🛠 Prerequisites

Before you begin, ensure you have met the following requirements:


Operating System: Windows, macOS, or Linux

Qt Framework: Version 5.15.2 or higher

Qt Creator: Integrated Development Environment for Qt

C++ Compiler: Compatible with your Qt version (e.g., GCC, MSVC)

GPS Data Files: Files containing latitude and longitude data (e.g., GPX, CSV)

📝 Installation

1. Clone the Repository
2. 
Open your terminal or command prompt and run:


bash

Copy code

git clone https://github.com/PritiPawar16/Map-using-QML.git

2. Navigate to the Project Directory
3. 
bash

Copy code

cd route-plotter

5. Install Dependencies
6. 
Ensure you have the necessary Qt components installed. If you haven't installed Qt yet, download it from the official Qt website.


8. Open the Project in Qt Creator
9. 
Launch Qt Creator.

Click on File > Open File or Project....

Select the RoutePlotter.pro file from the cloned repository.

11. Configure the Project
12. 
Choose the appropriate Qt kit for your environment.

Configure build settings as needed.

14. Build the Project
15. 
Click the Build button or press Ctrl + B to compile the project.


⚙️ Configuration

Setting Up Map Providers

The application uses map providers to render interactive maps. Ensure you have internet connectivity for map loading or configure offline map providers if necessary.


API Keys: If using services like Google Maps or OpenStreetMap, obtain the required API keys and configure them in the application's settings.


Map Settings: Navigate to the Settings tab within the application to select your preferred map provider and customize map display options.


Importing GPS Data

Supported Formats: The application supports GPS data in formats such as GPX and CSV.


Data Import:


Click on the Import button in the File menu.

Select your GPS data file.

The application will parse and display the route on the map and populate the data table.

🖥 Usage

Launching the Application

Run the Application: After building, click the Run button or press Ctrl + R in Qt Creator.


Main Interface: The main window displays the interactive map on the left and the data table on the right.


Importing and Plotting Routes

Import GPS File:


Click on File > Import file Data.

View Route:

The route will be plotted on the map.
The data table will display latitude, longitude, and calculated distances between points.
Calculating Distances
Automatic Calculation: The application automatically calculates the distance between consecutive GPS points upon data import.
Manual Calculation: Select two points in the data table and click on Calculate Distance to compute the distance between them.
Updating Data
Edit Entries:


Double-click on any cell in the data table to edit latitude or longitude values.

The map and distance calculations will update in real-time based on the changes.

Add/Remove Points:


Use the Add Point or Remove Point buttons to modify the route.

The map and table will reflect these changes immediately.

Exporting Data

Export to CSV: Click on File > Export Data to save the current table data as a CSV file for external use.

