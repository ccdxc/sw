import { ArrayChunkUtility } from '@app/common/ArrayChunkUtility';


describe('ArrayChunkUtility', () => {
  const data = [];
  const newData = [];
  const newData2 = [];
  beforeEach(() => {
    for (let index = 0; index < 300; index++) {
      data.push(index);
      newData.push(index + 'new');
      newData2.push(index + 'new2');
    }
  });

  it('should return correct data chunks', () => {
    const chunkUtility = new ArrayChunkUtility(data);
    let res = chunkUtility.requestChunk(50, 83);
    expect(res.length).toBe(33);
    expect(res[0]).toBe(50);
    expect(res[32]).toBe(82);
    expect(chunkUtility.length).toBe(300);

    // Update data but dont switch
    chunkUtility.updateData(newData);

    // Should return the exact same data
    res = chunkUtility.getLastRequestedChunk();
    expect(res.length).toBe(33);
    expect(res[0]).toBe(50);
    expect(res[32]).toBe(82);

    // Switch to new data
    chunkUtility.switchToNewData();

    res = chunkUtility.getLastRequestedChunk();
    expect(res.length).toBe(33);
    expect(res[0]).toBe('50new');
    expect(res[32]).toBe('82new');

    res = chunkUtility.requestChunk(200, 250);
    expect(res.length).toBe(50);
    expect(res[0]).toBe('200new');
    expect(res[49]).toBe('249new');

  });

  it('should support enabling/disabling instant update', () => {
    const chunkUtility = new ArrayChunkUtility(data, true);
    chunkUtility.updateData(newData);

    // should switch to data automatcially
    let res = chunkUtility.requestChunk(50, 83);
    expect(res.length).toBe(33);
    expect(res[0]).toBe('50new');
    expect(res[32]).toBe('82new');

    // shouldn't auto switch anymore
    chunkUtility.disableInstantUpdate();
    chunkUtility.updateData(data);
    res = chunkUtility.requestChunk(50, 83);
    expect(res.length).toBe(33);
    expect(res[0]).toBe('50new');
    expect(res[32]).toBe('82new');


    // update with instant switch
    chunkUtility.updateData(newData2, true);
    res = chunkUtility.requestChunk(50, 83);
    expect(res.length).toBe(33);
    expect(res[0]).toBe('50new2');
    expect(res[32]).toBe('82new2');

  });

  it('should through errors on invalid requests', () => {
    spyOn(console, 'error');

    const chunkUtility = new ArrayChunkUtility(data);
    chunkUtility.getLastRequestedChunk();
    expect(console.error).toHaveBeenCalledTimes(1);
    expect(console.error).toHaveBeenCalledWith('invalid chunk request');

    let res = chunkUtility.requestChunk(50, 83);
    chunkUtility.requestChunk(null, 2);
    expect(console.error).toHaveBeenCalledTimes(2);
    expect(console.error).toHaveBeenCalledWith('invalid chunk request');
    chunkUtility.requestChunk(0, null);
    expect(console.error).toHaveBeenCalledTimes(3);
    expect(console.error).toHaveBeenCalledWith('invalid chunk request');
    // The previous valid chunk request shouldn't have been overwritten
    res = chunkUtility.getLastRequestedChunk();
    expect(res.length).toBe(33);
  });

});
