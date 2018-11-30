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
  const serviceUtility = new HttpEventUtility<any>();
  const serviceUtilityFilter = new HttpEventUtility<any>(null, false,
    (object) => {
      return object.filter === 'include';
    }
  );
  const serviceUtilityConstructor = new HttpEventUtility<MockConstructor>(MockConstructor,
  );

  const serviceUtilitySingleton = new HttpEventUtility<any>(null, true);

  const createEvents = {
    events: [
      {
        type: 'Created',
        object: {
          meta: {
            name: 'obj1',
            'mod-time': '1'
          },
          filter: 'include'
        }
      },
      {
        type: 'Created',
        object: {
          meta: {
            name: 'obj2',
            'mod-time': '2'
          }
        }
      },
      {
        type: 'Created',
        object: {
          meta: {
            name: 'obj3',
            'mod-time': '3'
          }
        }
      },
      {
        type: 'Created',
        object: {
          meta: {
            name: 'obj4',
            'mod-time': '4'
          }
        }
      },
    ]
  };

  const createEvent = {
    events: [
      {
        type: 'Created',
        object: {
          meta: {
            name: 'obj1',
            'mod-time': '1'
          },
        }
      },
    ]
  };

  const putEvent = {
    events: [
      {
        type: 'Updated',
        object: {
          meta: {
            name: 'obj2',
            'mod-time': '1'
          },
        }
      },
    ]
  };

  const deleteEvent = {
    events: [
      {
        type: 'Deleted',
        object: {
          meta: {
            name: 'obj2',
            'mod-time': '1'
          }
        }
      },
    ]
  };

  const putAndCreateEvents = {
    events: [
      {
        type: 'Updated',
        object: {
          meta: {
            name: 'obj3',
            'mod-time': '6'
          }
        }
      },
      {
        type: 'Created',
        object: {
          meta: {
            name: 'obj5',
            'mod-time': '8'
          },
          filter: 'include'
        }
      },
    ]
  };

  it('Should process events to the same array', () => {
    const data = serviceUtility.array;
    serviceUtility.processEvents(createEvents);
    expect(data.length).toBe(4);
    expect(data[0].meta.name).toEqual('obj4');
    expect(data[1].meta.name).toEqual('obj3');
    expect(data[2].meta.name).toEqual('obj2');
    expect(data[3].meta.name).toEqual('obj1');

    serviceUtility.processEvents(deleteEvent);
    expect(data.length).toBe(3);
    expect(data[0].meta.name).toEqual('obj4');
    expect(data[1].meta.name).toEqual('obj3');
    expect(data[2].meta.name).toEqual('obj1');

    serviceUtility.processEvents(putAndCreateEvents);
    expect(data.length).toBe(4);
    expect(data[0].meta.name).toEqual('obj5');
    expect(data[1].meta.name).toEqual('obj3');
    expect(data[2].meta.name).toEqual('obj4');
    expect(data[3].meta.name).toEqual('obj1');
    expect(data[1].meta['mod-time']).toEqual('6');
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
    expect(data[0].meta.name).toEqual('obj5');
    expect(data[1].meta.name).toEqual('obj1');
  });

  it('Should use constructor on all items', () => {
    const data = serviceUtilityConstructor.array;
    serviceUtilityConstructor.processEvents(createEvents);
    expect(data.length).toBe(4);
    expect(data[0].data.meta.name).toEqual('obj4');
    expect(data[1].data.meta.name).toEqual('obj3');
    expect(data[2].data.meta.name).toEqual('obj2');
    expect(data[3].data.meta.name).toEqual('obj1');

    serviceUtilityConstructor.processEvents(deleteEvent);
    expect(data.length).toBe(3);
    expect(data[0].data.meta.name).toEqual('obj4');
    expect(data[1].data.meta.name).toEqual('obj3');
    expect(data[2].data.meta.name).toEqual('obj1');

    serviceUtilityConstructor.processEvents(putAndCreateEvents);
    expect(data.length).toBe(4);
    expect(data[0].data.meta.name).toEqual('obj5');
    expect(data[1].data.meta.name).toEqual('obj3');
    expect(data[2].data.meta.name).toEqual('obj4');
    expect(data[1].data.meta['mod-time']).toEqual('6');
    expect(data[3].data.meta.name).toEqual('obj1');
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
