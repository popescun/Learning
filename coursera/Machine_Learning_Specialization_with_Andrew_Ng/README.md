# Machine Learning Specialization 
Tutor: Andrew Ng

> # Warning: this repo contains only optional labs from the course. 

# Setup
The project is intended to be used with `venv`. 
This is also the configuration used by PyCharm projects.

Note: Coursera is using Jupyter notebooks. This project is mere another python setup alternative. 

## Install venv
```commandline
python -m venv .venv
```
Eventually PyCharm will ask you to configure the python interpreter. You need to select the one from `.venv`. 

## Activate venv
Make sure the local `.venv` is activated.
```commandline
source .venv/bin/activate
```
or reopen the terminal in PyCharm. This will automatically activate the `.venv`.

## Install required packages
```commandline
pip install -r requirements.txt
```

### Install required tools
Some packages used in labs may require additional tools from bellow to be present on your machine:
On `MacOs`:
```commandline
 brew install graphviz
 brew install libomp
```

## Save requirements
```commandline
python -m pip freeze > requirements.txt
```
