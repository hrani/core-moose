# hdfutil.py --- 
# 
# Filename: hdfutil.py
# Description: 
# Author: 
# Maintainer: 
# Created: Thu Aug 23 17:34:55 2012 (+0530)
# Version: 
# Last-Updated: Sat Aug 25 12:31:00 2012 (+0530)
#           By: subha
#     Update #: 265
# URL: 
# Keywords: 
# Compatibility: 
# 
# 

# Commentary: 
# 
# Utility function to save data in hdf5 format
# 
# 

# Change log:
# 
# 
# 
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street, Fifth
# Floor, Boston, MA 02110-1301, USA.
# 
# 

# Code:

import moose as moose__
import numpy as np
import h5py as h5
import time

cpptonp = {
    'int': 'i4',
    'long': 'i8',
    'bool': 'b',
    'unsigned int': 'u4',
    'unsigned long': 'u8',
    'float': 'f4',
    'double': 'f8',
    'string': 'S1024'}
    
size_step=256
def savestate(filename=None):
    """Dump the state of MOOSE in an hdf5 file.
    
    The file will have a data set for each class.
    Each such dataset will be a column of field values.
    """    
    if filename is None:
        filename = 'moose_session_' + time.strftime('%Y%m%d_%H%M%S') + '.hdf5'
    with h5.File(filename, 'w') as fd:
        root = fd.create_group('/elements')
        meta = fd.create_group('/metadata')
        typeinfo_dataset = meta.create_dataset('typeinfo', shape=(size_step,), dtype=[('path', 'S1024'), ('class', 'S64'), ('dims', 'S64'), ('parent', 'S1024')], compression='gzip', compression_opts=6)
        typeinfo = []
        class_dataset_dict = {}
        class_count_dict = {}
        class_array_dict = {}
        objcount = 0
        for obj in moose__.wildcardFind("/##"):
            if obj.path.startswith('/Msg') or obj.path.startswith('/class') or obj.class_ == 'Table' or obj.class_ == 'TableEntry':
                continue
            print 'Processing:', obj.path, obj.class_
            typeinfo.append((obj.path, obj.class_, str(obj.shape), obj[0].parent.path))
            objcount += 1
            if len(typeinfo) == size_step:
                typeinfo_dataset.resize(objcount)
                typeinfo_dataset[objcount - size_step: objcount] = np.rec.array(typeinfo, typeinfo_dataset.dtype)
                typeinfo = []
            # If we do not yet have dataset for this class, create one and keep it in dict
            if obj.class_ not in class_dataset_dict:
                print 'Creating entries for class:', obj.class_
                fielddict = moose__.getFieldDict(obj.class_, 'valueFinfo')
                print fielddict
                keys = sorted(list(fielddict.keys()))
                fields = [] # [('path', 'S1024')]
                for fieldname in keys:
                    ftype = fielddict[fieldname]
                    if ftype in cpptonp:
                        fields.append((fieldname, cpptonp[ftype]))
                    elif ftype == 'Id' or ftype == 'ObjId':
                        fields.append((fieldname, 'S1024'))
                # print fields
                ds = root.create_dataset(obj.class_, shape=(size_step,), dtype=fields, compression='gzip', compression_opts=6)
                class_dataset_dict[obj.class_] = ds
                class_array_dict[obj.class_] = []
                class_count_dict[obj.class_] = 0
            # Lookup the dataset for the class this object belongs to
            ds = class_dataset_dict[obj.class_]
            for entry in obj:
                fields = []
                print entry.path,
                for f in ds.dtype.names:
                    print 'getting field:', f
                    entry.getField(f)
                fields = [f.path if isinstance(f, moose__.ematrix) or isinstance(f, moose__.element) else f for f in fields]
                class_array_dict[obj.class_].append(fields)
                # print 'fields:'
                # print fields
                # print 'length:', len(class_array_dict[obj.class_])
                class_count_dict[obj.class_] += 1
                if class_count_dict[obj.class_] == size_step:
                    oldlen = ds.len()
                    if oldlen <= class_count_dict[obj.class_]:
                        ds.resize((class_count_dict[obj.class_]))
                    ds[oldlen: class_count_dict[obj.class_]] = np.rec.array(class_array_dict[obj.class_], dtype=ds.dtype)
                    class_array_dict[obj.class_] = []
        for classname in class_array_dict:
            ds = class_dataset_dict[classname]
            ds.resize((class_count_dict[classname], ))
            if len(class_array_dict[classname]) > 0:
                start = class_count_dict[classname] - len(class_array_dict[classname])
                ds[start:] = np.rec.array(class_array_dict[classname], dtype=ds.dtype)

        if len(typeinfo) > 0:
            typeinfo_dataset.resize((objcount,))
            typeinfo_dataset[objcount-len(typeinfo): objcount] = np.rec.array(typeinfo, dtype=typeinfo_dataset.dtype)

def restorestate(filename):    
    wfields = {}
    for cinfo in moose__.element('/classes').children:
        cname = cinfo[0].name
        wfields[cname] = [f[len('set_'):] for f in moose__.getFieldNames(cname, 'destFinfo') 
                          if f.startswith('set_') and f not in ['set_this', 'set_name', 'set_lastDimension', 'set_runTime'] and not f.startswith('set_num_')]
    with h5.File(filename, 'r') as fd:
        typeinfo = fd['/metadata/typeinfo'][:]
        classdict = {}
        dimsdict = dict(zip(typeinfo['path'], typeinfo['dims']))
        classdict = dict(zip(typeinfo['path'], typeinfo['class']))
        parentdict = dict(zip(typeinfo['path'], typeinfo['parent']))
        sorted_paths = sorted(typeinfo['path'], key=lambda x: x.count('/'))
        for path in sorted_paths:
            name = path.rpartition('/')[-1].partition('[')[0]
            moose__.ematrix(parentdict[path] + '/' + name, eval(dimsdict[path]), classdict[path])
        for key in fd['/elements']:
            dset = fd['/elements/'][key][:]
            fieldnames = dset.dtype.names
            for ii in range(len(dset)):
                obj = moose__.element(dset['path'][ii])
                for f in wfields[obj.class_]:
                    obj.setField(f, dset[f][ii])

# 
# hdfutil.py ends here