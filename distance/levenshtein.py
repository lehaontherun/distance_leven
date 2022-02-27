from array import array


def levenshtein(flow1, flow2, normalized=False, max_distance=-1):
  if normalized:
      return nlevenshtein(flow1, flow2, max_distance=max_distance)
  
  if flow1 == flow2:
      return 0
  
  len1, len2 = len(flow1), len(flow2)
  if max_distance >= 0 and abs(len1-len2) > max_distance:
      return - 1
  if len1 == 0:
      return len2
  if len2 == 0:
      return len1
  if len1 < len2:
      len1, len2 = len2, len1
      flow1, flow2 = flow2, flow1
      
  column = array('L', range(len2 + 1))
  
  for elem in range(1, len1 + 1):
    column[0] = elem
    new = elem - 1
    for other_elem in range(1, len2 + 1):
      old = column[other_elem]
      cost = int(flow1[elem - 1] != flow2[other_elem - 1])
      column[other_elem] = min(column[other_elem] +1, column[other_elem - 1] + 1, new + cost)
      new = old
    if max_distance >=0 and min(column) > max_distance:
      return -1
    
  if max_distance >= 0 and column[len2] > max_distance:
    return -1
  return column[len2]


def nlevenshtein(flow1, flow2, func=1):
  if flow1 == flow2:
    return 0.0
  len1, len2 = len(flow1), len(flow2)
  if len1 ==0 or len2 == 0:
    return 1.0
  if len1 < len2:
    len1, len2 = len2, len1
    flow1, flow2 = flow2, flow1
    
  if func == 1:
    return levenshtein(flow1, flow2) / float(len1)
  
  column = array('L', range(1, len2 + 1))
  length = array('L', range(1, len2 + 1))
  
  for elem in range(1, len1 + 1):
    
    column[0] = length[0] = elem
    new = lnew = elem - 1
    
    for other_elem in range(1, len2 + 1):
      
      old = column[other_elem]
      ic = column[other_elem - 1] + 1
      dc = column[other_elem] + 1
      rc = new + (flow1[elem - 1] != flow2[other_elem - 1])
      column[other_elem] = min(ic, dc, rc)
      new = old
      
      lold = length[other_elem]
      lic = length[other_elem - 1] + 1 if ic == column[other_elem] else 0
      ldc = length[other_elem] + 1 if dc == column[other_elem] else 0
      lrc = lnew + 1 if rc == column[other_elem] else 0
      length[other_elem] = max(ldc, lic, lrc)
      lnew = lold
      
  return column[other_elem] / float(length[other_elem])
    
