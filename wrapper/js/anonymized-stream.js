export default class AnonymizedStream {
  constructor (original, chunkSize) {
    this.size = original.size
    this._original = original
    this._changes = []
    this._start = 0
    this._chunkSize = chunkSize
    this._done = false
  }


  // Public JS API (used by UI and WASM code)

  static create (file, chunkSize, path) {
    const stream = new AnonymizedStream(file, chunkSize)
    const filename = path ? path + '/' + file.name : file.name
    AnonymizedStream._files[filename] = stream
    return stream
  }

  static exists (filename) {
    return Object.prototype.hasOwnProperty.call(AnonymizedStream._files, filename)
  }

  static retrieve (filename, path) {
    const filepath = path ? path + '/' + filename : filename
    return AnonymizedStream._files[filepath]
  }

  static destroy (filename, path) {
    const filepath = path ? path + '/' + filename : filename
    delete AnonymizedStream._files[filepath]
  }

  async anonymize () {
    const awi = Module.cwrap("wsi_anonymize", "number", ["string", "string", "number", "number"], { async: true })
    const result = await awi(this._original.name, 'newlabel', false, false)
    if (result != 0) {
      throw Error("Anonymization failed");
    }
  }

  addChanges (data, offset) {
    this._changes.push({
      start: Number(offset),
      size: data.byteLength,
      data: data
    })
  }

  async getAnonymizedChunk (offset, size) {
    const slice = this._original.slice(Number(offset), Number(offset) + size)
    const sliceData = await slice.arrayBuffer()
    this._applyChanges(sliceData, Number(offset))
    return sliceData
  }


  // ReadableStreamDefaultReader compliant API to be used in upload client

  async read () {
    if (this._start === this.size) {
      const value = undefined
      const done = true
      return Promise.resolve({ value, done })
    }
    let end = Math.min(this._start + this._chunkSize, this.size)
    return new Promise((resolve, reject) => {
      const slice = this._original.slice(this._start, end)
      slice.arrayBuffer().then(buffer => {
        this._applyChanges(buffer, this._start)
        this._start = end
        const value = new Blob([buffer])
        const done = false
        resolve({ value, done })
      })
    })
  }


  // private API

  _applyChanges (buffer, offset) {
    const bufferSize = buffer.byteLength
    const end = offset + bufferSize
    for (let i = 0; i < this._changes.length; ++i) {
      const changeStart = this._changes[i].start
      const changeSize = this._changes[i].size
      const changeEnd = changeStart + changeSize
      if ((changeStart < offset && changeEnd > offset) || (changeStart >= offset && changeStart < end)) {
        console.log('Applying changes...')
        const changeOffset = offset - changeStart
        const frontClip = Math.max(0, changeOffset)
        const tailClip = Math.max(0, (changeStart + changeSize) - (offset + bufferSize))
        const viewSize = Math.min(bufferSize, changeSize - frontClip - tailClip)
        const changeSourceView = new Uint8Array(this._changes[i].data, frontClip, viewSize)
        const bufferOffset = changeOffset < 0 ? Math.abs(changeOffset) : 0
        const changeTargetView = new Uint8Array(buffer, bufferOffset, viewSize)
        changeTargetView.set(changeSourceView)
      }
    }
  }
}

AnonymizedStream._files = {}
