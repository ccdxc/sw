import { MetricsUtility } from '@app/common/MetricsUtility';


describe('MetricsUtility', () => {

  it('maxObjQueryMerge', () => {
    let currData: any;
    let newData: any;
    let ret;

    newData = {
      series: [
        {}
      ]
    };
    ret = MetricsUtility.maxObjQueryMerge(currData, newData);
    expect(ret).toEqual(currData);

    newData = {
      series: [
        {
          values: [10]
        }
      ]
    };
    ret = MetricsUtility.maxObjQueryMerge(currData, newData);
    expect(ret).toEqual(newData);

    currData = {
      series: [
        {
          values: [6]
        }
      ]
    };
    ret = MetricsUtility.maxObjQueryMerge(currData, newData);
    expect(ret).toEqual(newData);
  });

  it('maxObjUtility', () => {
    let data;
    let fieldName;
    let ret;

    ret = MetricsUtility.maxObjUtility(data, fieldName);
    expect(ret).toBeNull();

    data = {
      series: [
        {
          tags: {
            reporterID: 'r1',
          },
          columns: [
            'f1', 'f2', 'f3'
          ],
          values: [[1, 2, 3]]
        }
      ]
    };
    // Fieldname is blank
    ret = MetricsUtility.maxObjUtility(data, fieldName);
    expect(ret).toBeNull();

    fieldName = '';
    ret = MetricsUtility.maxObjUtility(data, fieldName);
    expect(ret).toBeNull();

    fieldName = 'invalidField';
    ret = MetricsUtility.maxObjUtility(data, fieldName);
    expect(ret).toBeNull();

    fieldName = 'f2';
    ret = MetricsUtility.maxObjUtility(data, fieldName);
    expect(ret).toBeTruthy();
    expect(ret.name).toBe('r1');
    expect(ret.max).toBe(2);

    data = {
      series: [
        {
          tags: {
            reporterID: 'r1',
          },
          columns: [
            'f1', 'f2', 'f3'
          ],
          values: [[1, 2, 3]]
        },
        {
          tags: {
            reporterID: 'r2',
          },
          columns: [
            'f1', 'f2', 'f3'
          ],
          values: [[4, 5, 6]]
        },
        {
          tags: {
            reporterID: 'r3',
          },
          columns: [
            'f1', 'f2', 'f3'
          ],
          values: [[4, 1, 6]]
        },
        {
          tags: {
            reporterID: 'r4',
          },
          columns: [
            'f1', 'f3'
          ],
          values: [[7, 9]]
        }
      ]
    };
    fieldName = 'f2';
    ret = MetricsUtility.maxObjUtility(data, fieldName);
    expect(ret).toBeTruthy();
    expect(ret.name).toBe('r2');
    expect(ret.max).toBe(5);
  });

  it('responseHasData', () => {
    let input;
    let ret;

    ret = MetricsUtility.responseHasData(input);
    expect(ret).toBeFalsy();

    input = null;
    ret = MetricsUtility.responseHasData(input);
    expect(ret).toBeFalsy();

    input = {};
    ret = MetricsUtility.responseHasData(input);
    expect(ret).toBeFalsy();

    input = { results: [] };
    ret = MetricsUtility.responseHasData(input);
    expect(ret).toBeFalsy();

    input = {
      results: [
        {
          statementId: ''
        }
      ]
    };
    ret = MetricsUtility.responseHasData(input);
    expect(ret).toBeFalsy();

    input = {
      results: [
        {
          series: []
        }
      ]
    };
    ret = MetricsUtility.responseHasData(input);
    expect(ret).toBeFalsy();

    input = {
      results: [
        {
          series: [
            {
              columns: []
            }
          ]
        }
      ]
    };
    ret = MetricsUtility.responseHasData(input);
    expect(ret).toBeFalsy();

    input = {
      results: [
        {
          series: [
            {
              values: []
            }
          ]
        }
      ]
    };
    ret = MetricsUtility.responseHasData(input);
    expect(ret).toBeFalsy();

    input = {
      results: [
        {
          series: [
            {
              values: [[10]]
            }
          ]
        }
      ]
    };
    ret = MetricsUtility.responseHasData(input);
    expect(ret).toBeTruthy();
  });

});
