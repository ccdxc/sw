import csv
import operator

def convert_csv_to_html(csv_file, html_file, title):
    reader = csv.reader(open(csv_file))

    # Create the HTML file
    f_html = open(html_file,"w");
    f_html.write('<title><%s></title>' % title)

    row_count = 0
    f_html.write('<table>')
    for row in reader:
        f_html.write('<tr>')
        for column in row:
            if row_count:
                f_html.write('<td>' + column + '</td>');
            else:
                f_html.write('<th>' + column + '</th>');
        f_html.write('</tr>')
        row_count += 1
    f_html.write('</table>')
    f_html.close()
    
def combine_dicts(a, b, op=operator.add):
    return dict(a.items() + b.items() +
        [(k, op(a[k], b[k])) for k in set(b) & set(a)])

class RangeDict(dict):
    def __getitem__(self, item):
        if type(item) != xrange:
            for key in self.keys():
                if item in key:
                    return self.get(key)
        else:
            return super().__getitem__(item)