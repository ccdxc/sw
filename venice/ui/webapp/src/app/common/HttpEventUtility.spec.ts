import { HttpEventUtility } from '@app/common/HttpEventUtility';

class MockConstructor {
  data: any;
  meta: any;

  constructor(data) {
    this.data = data;
    this.meta = {
      name: data.meta.name
    };
  }
}

describe('HttpEventUtility', () => {
  const serviceUtility = new HttpEventUtility();
  const serviceUtilityFilter = new HttpEventUtility(null, false,
    (object) => {
      return object.filter === 'include';
    }
  );
  const serviceUtilityConstructor = new HttpEventUtility(MockConstructor,
  );

  const serviceUtilitySingleton = new HttpEventUtility(null, true);

  const createEvents = {
    result: {
      Events: [
        {
          Type: 'Created',
          Object: {
            meta: {
              name: 'obj1',
              'mod-time': '1'
            },
            filter: 'include'
          }
        },
        {
          Type: 'Created',
          Object: {
            meta: {
              name: 'obj2',
              'mod-time': '2'
            }
          }
        },
        {
          Type: 'Created',
          Object: {
            meta: {
              name: 'obj3',
              'mod-time': '3'
            }
          }
        },
      ]
    }
  };

  const createEvent = {
    result: {
      Events: [
        {
          Type: 'Created',
          Object: {
            meta: {
              name: 'obj1',
              'mod-time': '1'
            },
          }
        },
      ]
    }
  };

  const putEvent = {
    result: {
      Events: [
        {
          Type: 'Updated',
          Object: {
            meta: {
              name: 'obj2',
              'mod-time': '1'
            },
          }
        },
      ]
    }
  };

  const deleteEvent = {
    result: {
      Events: [
        {
          Type: 'Deleted',
          Object: {
            meta: {
              name: 'obj2',
              'mod-time': '1'
            }
          }
        },
      ]
    }
  };

  const putAndCreateEvents = {
    result: {
      Events: [
        {
          Type: 'Updated',
          Object: {
            meta: {
              name: 'obj3',
              'mod-time': '6'
            }
          }
        },
        {
          Type: 'Created',
          Object: {
            meta: {
              name: 'obj4',
              'mod-time': '8'
            },
            filter: 'include'
          }
        },
      ]
    }
  };

  it('Should process events to the same array', () => {
    const data = serviceUtility.array;
    serviceUtility.processEvents(createEvents);
    expect(data.length).toBe(3);
    expect(data[0].meta.name).toEqual('obj1');
    expect(data[1].meta.name).toEqual('obj2');
    expect(data[2].meta.name).toEqual('obj3');

    serviceUtility.processEvents(deleteEvent);
    expect(data.length).toBe(2);
    expect(data[0].meta.name).toEqual('obj1');
    expect(data[1].meta.name).toEqual('obj3');

    serviceUtility.processEvents(putAndCreateEvents);
    expect(data.length).toBe(3);
    expect(data[0].meta.name).toEqual('obj1');
    expect(data[1].meta.name).toEqual('obj3');
    expect(data[1].meta['mod-time']).toEqual('6');
    expect(data[2].meta.name).toEqual('obj4');
  });

  it('Should filter events', () => {
    const data = serviceUtilityFilter.array;
    serviceUtilityFilter.processEvents(createEvents);
    expect(data.length).toBe(1);
    expect(data[0].meta.name).toEqual('obj1');

    serviceUtilityFilter.processEvents(deleteEvent);
    expect(data.length).toBe(1);
    expect(data[0].meta.name).toEqual('obj1');

    serviceUtilityFilter.processEvents(putAndCreateEvents);
    expect(data.length).toBe(2);
    expect(data[0].meta.name).toEqual('obj1');
    expect(data[1].meta.name).toEqual('obj4');
  });

  it('Should use constructor on all items', () => {
    const data = serviceUtilityConstructor.array;
    serviceUtilityConstructor.processEvents(createEvents);
    expect(data.length).toBe(3);
    expect(data[0].data.meta.name).toEqual('obj1');
    expect(data[1].data.meta.name).toEqual('obj2');
    expect(data[2].data.meta.name).toEqual('obj3');

    serviceUtilityConstructor.processEvents(deleteEvent);
    expect(data.length).toBe(2);
    expect(data[0].data.meta.name).toEqual('obj1');
    expect(data[1].data.meta.name).toEqual('obj3');

    serviceUtilityConstructor.processEvents(putAndCreateEvents);
    expect(data.length).toBe(3);
    expect(data[0].data.meta.name).toEqual('obj1');
    expect(data[1].data.meta.name).toEqual('obj3');
    expect(data[1].data.meta['mod-time']).toEqual('6');
    expect(data[2].data.meta.name).toEqual('obj4');
  });

  it('should handle updates if service is singleton', () => {
    const data = serviceUtilitySingleton.array;
    serviceUtilitySingleton.processEvents(createEvent);
    expect(data.length).toBe(1);
    expect(data[0].meta.name).toEqual('obj1');

    // Even though update contains different name,
    // it should update the one object we have

    serviceUtilitySingleton.processEvents(putEvent);
    expect(data.length).toBe(1);
    expect(data[0].meta.name).toEqual('obj2');
  });
});
