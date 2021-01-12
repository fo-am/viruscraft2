import json
import base64

# stuff all the scheme code into the ditto js file
# so that it can be loaded locally

pre = "static-pre.html"
target = "static.html"

# more time and I would go through searching for loads to
# do this automatically - todo
code = ["scm/game.jscm"]

resources = [
    "scm/base.jscm",
    "scm/maths.jscm",
    "scm/random.jscm",
    "scm/local-storage.jscm",
    "scm/nightjar.jscm",

    "scm/ordered-list.jscm",
    "scm/particles.jscm",
    "scm/animation.jscm",
    "scm/entity.jscm",
    "scm/entity-list.jscm",
    "scm/entity-renderer.jscm",
    "scm/windows.jscm",
    "scm/wasp.jscm",
    "scm/cell.jscm",
    "scm/event.jscm",
    "scm/storm-event.jscm",
    "scm/pestcontrol-event.jscm",
    "scm/wildflower-event.jscm",
    "scm/manicured-event.jscm",
    "scm/ant-event.jscm",
    "scm/organic-event.jscm",
    "scm/swatted-event.jscm",
    "scm/ants.jscm",
    "scm/hoverfly.jscm",
    "scm/hoverfly-event.jscm",
    "scm/scores.jscm",
    "scm/nest.jscm",
    "scm/menu.jscm",
    "scm/info.jscm",
    "scm/demo.jscm",
    "scm/score-popup.jscm"
]

################################################

def load_from_file(fn):
    with open(fn, 'r') as myfile:
        return myfile.read()

def load_from_files(fnl):
    ret = ""
    for fn in fnl:
        ret+=load_from_file(fn)
    return ret

def base64_from_file(fn):
    with open(fn, "rb") as f:
        return base64.b64encode(f.read())
        
def insert_code(target_data,target,scm):
    scm = scm.replace("\n","\\n\\\n")
    scm = scm.replace("'","\\'")
    return target_data.replace(target,scm)

def build_resources(resource_files):
    res = {}
    for fn in resource_files:
        if fn.endswith(".png") or fn.endswith(".jpg"):
            res[fn]=base64_from_file(fn)
        else:
            res[fn]=load_from_file(fn)
    return json.dumps(res)

###################################################

pre_data=load_from_file(pre)
target_data=pre_data

target_data=insert_code(target_data,"{{SYNTAX}}",load_from_file("scm/syntax.jscm"))
target_data=insert_code(target_data,"{{CODE}}",load_from_files(code))
target_data=insert_code(target_data,"{{RESOURCES}}",build_resources(resources))

with open(target, 'w') as myfile:
    myfile.write(target_data)
