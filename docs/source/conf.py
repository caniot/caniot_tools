# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Caniot Tool'
copyright = '2022, caniot team'
author = 'caniot team'
release = '1.0.1'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx_rtd_theme",
    'sphinx.ext.autosectionlabel',
    'sphinx_tabs.tabs',
    'sphinxcontrib.video',
]
templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
# html_static_path = ['_static']
html_favicon = './images/favicon.png'
html_title = 'Caniot Tool'
html_logo = './images/twitter_header_photo_2.png'
html_theme_options = {
    'logo_only': False,
    'display_version': True,
}