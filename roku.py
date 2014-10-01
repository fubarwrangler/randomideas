#!/usr/bin/python

import time
import requests
from xml.etree import ElementTree

class RokuApp(object):
    def __init__(self, app_id, name, version, remote=None):
        self.id = app_id
        self.name = name
        self.version = version
        self._remote = remote
    def __repr__(self):
        cls = self.__class__.__name__
        values = {k: repr(v) for k, v in self.__dict__.iteritems()}
        return "{0}({id}, {name}, {version})".format(cls, **values)
    def launch(self):
        self._remote.launch(self.id)

class RokuRemote(object):
    def __init__(self, location):
        self.location = location
    
    def __repr__(self):
        return "{0}({1})".format(self.__class__.__name__, repr(self.location))
    
    def _call(self, method, path, params=None):
        url = "".join([self.location, path])
        return requests.request(method, url, params=params).text
    
    def keypress(self, key, count=1, delay=0.05):
        for x in xrange(count):
            self._call("post", "/keypress/{0}".format(key))
            time.sleep(delay)

    def right(self, count=1):
        self.keypress('right', count)

    def left(self, count=1):
        self.keypress('left', count)

    def up(self, count=1):
        self.keypress('up', count)

    def down(self, count=1):
        self.keypress('down', count)

    def keydown(self, key):
        self._call("post", "/keydown/{0}".format(key))

    def keyup(self, key):
        self._call("post", "/keyup/{0}".format(key))
    
    def query_apps(self):
        apps_xml = ElementTree.fromstring(self._call("get", "/query/apps"))
        apps = {}
        for app in apps_xml.getchildren():
            a = app.attrib
            apps[a["id"]] = RokuApp(a["id"], app.text, a["version"], self)
        return apps

    def launch(self, app, params=None):
        self._call("post", "/launch/{0}".format(app), params=params)
    
    def send_input(self, params):
        self._call("post", "/input", params=params)
    
    def type_string(self, string):
        for letter in string:
            self.keypress("Lit_{0}".format(letter))

# a-z, _(space), switch($), backspace(!), clear(*)
letters = 'abcdefghijklmnopqrstuvwxyz $!*'

# 1-9, 0, switch($), backspace(!), clear(*)
numbers = '1234567890$!*'

# My sign, +0 is positive
sign = lambda i: -1 if i < 0 else 1


def best_move(o, n, ref):
    dist = mycompare(o, n, ref)
    print 'comp: %d' % dist,
    size = len(ref)
    if abs(dist) > size / 2:
        dist = (abs(dist) - len(ref)) * sign(dist)
    return dist

def mycompare(o, n, set):
    return set.index(n) - set.index(o)


def set_position(o, n, r):
    print '%c -> %c' % (o, n),

    def move(dist):
        # Future me: I'm sorry
        getattr(r, ['left', 'right'][dist > 0])(abs(dist))

    # As long as they're in the same character class (digit or ~digit)
    if not (o.isdigit() ^ n.isdigit()):

        dist = best_move(o, n, numbers if o.isdigit() else letters)
        print 'move: %d' % dist

        move(dist)

    else:
        dist = best_move(o, '$', numbers if o.isdigit() else letters)
        print 'move to switch %d' % dist,
        move(dist)
        r.keypress('select')
        dist = best_move('$', n, numbers if n.isdigit() else letters)
        print 'move to final %d' % dist
        move(dist)


def youtube_type(roku, string, starting_key = 'a'):
    last = starting_key
    for char in string.strip():
        set_position(last, char, roku)
        roku.keypress('select')
        last = char


roku = RokuRemote('http://192.168.1.128:8060')

youtube_type(roku, raw_input('Search: '))

