import { Utility } from '@app/common/Utility';

/**
 * This utility class allows for easily chunking data and
 * handling incoming data for PrimeNG Turbo table.
 *
 * Any data passed to the table will instantly be rendered. For efficiency,
 * we use lazy loading and simulate the chunking of the data. This utility
 * holds all the data, and requestChunk should be used to give primeNG only the chunks
 * it requests during the lazyLoad Event.
 *
 * This utility also handles holding data updates. When data comes in, the user
 * may not be ready to see it. Once the user is ready, we can use switchToNewData
 * to change the reference that is being used when data is requested.
 */
export class ArrayChunkUtility {
  currentDataArray: any[] = [];
  newDataArray: any[];
  lastRequestedIndex = {
    start: null,
    stop: null,
  };
  lastRequestedSort = {
    field: null,
    order: null
  };
  // Whether to instantly update the currentDataArray on updateData
  instantUpdate: boolean;

  public get length(): number {
    return this.currentDataArray.length;
  }

  /**
   *
   * @param data          initial data
   *
   * @param instantUpdate Whether to automatically update
   *                      currentDataArray instead of holding the data
   */
  constructor(data: any[], instantUpdate: boolean = false) {
    this.instantUpdate = instantUpdate;
    this.currentDataArray = Utility.getLodash().cloneDeep(data);
  }

  /**
   *
   * @param field Field to sort by
   * @param order 1 for asc, -1 for dec
   */
  sort(field: string, order: number) {
    if (order !== 1 && order !== -1) {
      console.error('Invalid sort order given');
      return;
    }
    this.lastRequestedSort = {
      field: field,
      order: order
    };
    if (field == null || field === '') {
      // Assume contents of the array are primitive types
      // since field wasn't given
      this.currentDataArray.sort((a, b) => {
        // if order is 1, it will be ascending
        // if order is -1, it will be descending
        if (a < b) {
          return -1 * order;
        } else if (a > b) {
          return 1 * order;
        }
        return 0;
      });
    } else {
      const _ = Utility.getLodash();
      this.currentDataArray.sort((a, b) => {
        if (_.get(a, field, 0) < _.get(b, field, 0)) {
          return -1 * order;
        } else if (_.get(a, field, 0) > _.get(b, field, 0)) {
          return 1 * order;
        }
        return 0;
      });
    }
  }

  /**
   * Returns a chunk of the current data
   * @param start
   * @param stop
   */
  requestChunk(start: number, stop: number): ReadonlyArray<any> {
    if (start == null || stop == null) {
      console.error('invalid chunk request');
      return [];
    }
    this.lastRequestedIndex = {
      start: start,
      stop: stop,
    };
    return this.currentDataArray.slice(start, stop);
  }

  /**
   * Returns the last request chunk on the current data source.
   *
   * This is useful for using after switching
   * data sources, and needing to replicate the last request
   * but with the new data.
   */
  getLastRequestedChunk(): ReadonlyArray<any> {
    const start = this.lastRequestedIndex.start;
    const stop = this.lastRequestedIndex.stop;
    return this.requestChunk(start, stop);
  }

  /**
   * If there are updates available, it will switch to it
   */
  switchToNewData() {
    if (this.newDataArray != null) {
      this.currentDataArray = this.newDataArray;
      this.newDataArray = null;
      if (this.lastRequestedSort.order != null) {
        this.sort(this.lastRequestedSort.field, this.lastRequestedSort.order);
      }
    }
  }

  updateData(data, switchNow = false) {
    this.newDataArray = Utility.getLodash().cloneDeep(data);
    if (this.instantUpdate || switchNow) {
      this.switchToNewData();
    }
  }

  enableInstantUpdate() {
    this.instantUpdate = true;
    this.switchToNewData();
  }

  disableInstantUpdate() {
    this.instantUpdate = false;
  }

  hasUpdate() {
    return this.newDataArray != null;
  }
}
