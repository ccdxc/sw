import { Utility } from '@app/common/Utility';


/**
 * Create a new object for every watch endpoint in use.
 * Only checks by object name for uniqueness
 */
enum EventTypes {
  create = 'Created',
  update = 'Updated',
  delete = 'Deleted',
}

export class HttpEventUtility {
  private dataArray = [];
  private dataMapping = {};
  private filter: (object: any) => boolean;

  /**
   *
   * @param filter If the filter returns false for an object,
   *               it won't be added to the array
   */
  constructor(filter: (object: any) => boolean = null) {
    if (filter) {
      this.filter = filter;
    }
  }

  /**
   * Can be used by other components as an efficient way to
   * check if the array has changed
   */
  public static trackBy(index: number, item: any) {
    return item.meta.name + ' - ' + item.meta['mod-time'];
  }

  public processEvents(eventChunk) {
    if (eventChunk.result == null) {
      console.log('event chunk was blank');
      return;
    }
    const events = eventChunk.result.Events;
    events.forEach(event => {
      const obj = event.Object;
      const objName = obj.meta.name;
      if (this.filter != null && !this.filter(obj)) {
        return;
      }
      let index;
      switch (event.Type) {
        case EventTypes.create:
          index = this.dataArray.length;
          this.dataArray.push(obj);
          this.dataMapping[objName] = index;
          break;
        case EventTypes.delete:
          index = this.dataMapping[objName];
          this.dataArray.splice(index, 1);
          delete this.dataMapping[objName];
          // Decrement index of every element after
          // the one we removed
          for (const key in this.dataMapping) {
            if (this.dataMapping.hasOwnProperty(key)) {
              const value = this.dataMapping[key];
              if (value > index) {
                this.dataMapping[key] = value - 1;
              }
            }
          }
          break;
        case EventTypes.update:
          index = this.dataMapping[objName];
          this.dataArray[index] = obj;
          break;
        default:
          break;
      }
    });

    return this.dataArray;
  }

  public get array(): ReadonlyArray<any> {
    return this.dataArray;
  }

}
