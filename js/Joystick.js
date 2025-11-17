// eslint-disable-next-line no-unused-vars
class Joystick {
  constructor({area, document, fractionDigits, handle}) {
    this.areaRect = null;
    this.isMoving = false;
    this.onUpdateCallback = null;
    this.x = 0;
    this.y = 0;

    this.area = area;
    this.fractionDigits = fractionDigits;
    this.handle = handle;

    area.addEventListener('mousedown', (e) => this.pointerDown(e.clientX, e.clientY));
    document.addEventListener('mousemove', (e) => this.pointerMove(e.clientX, e.clientY));
    document.addEventListener('mouseup', () => this.pointerUp());

    area.addEventListener('touchstart', (e) => this.pointerDown(e.touches[0].clientX, e.touches[0].clientY));
    document.addEventListener('touchmove', (e) => this.pointerMove(e.touches[0].clientX, e.touches[0].clientY));
    document.addEventListener('touchend', () => this.pointerUp());
  }

  onUpdate(callback) {
    this.onUpdateCallback = callback;
  }

  getX() {
    return this.x;
  }

  getY() {
    return this.y;
  }

  pointerDown(clientX, clientY) {
    this.areaRect = this.area.getBoundingClientRect();
    this.isMoving = true;

    this.handle.classList.add('moving');
    this.moveHandle(clientX, clientY);
  }

  pointerMove(clientX, clientY) {
    if (this.isMoving) {
      this.moveHandle(clientX, clientY);
    }
  }

  pointerUp() {
    this.areaRect = null;
    this.isMoving = false;

    this.handle.classList.remove('moving');
    this.resetHandle();
  }

  moveHandle(clientX, clientY) {
    const left = Math.min(Math.max(0, clientX - this.areaRect.left), this.areaRect.width);
    const top = Math.min(Math.max(0, clientY - this.areaRect.top), this.areaRect.height);

    this.handle.style.left = `${left}px`;
    this.handle.style.top = `${top}px`;

    const x = (2 * left / this.areaRect.width) - 1;
    const y = 1 - (2 * top / this.areaRect.height);

    this.updatePosition(x, y);
  }

  resetHandle() {
    this.handle.removeAttribute('style');
    this.updatePosition(0, 0);
  }

  updatePosition(x, y) {
    // Apply rounding before comparison to ignore changes below precision threshold.
    const newX = x === 0 ? 0 : parseFloat(x.toFixed(this.fractionDigits));
    const newY = y === 0 ? 0 : parseFloat(y.toFixed(this.fractionDigits));
    let isUpdated = false;

    if (newX !== this.x) {
      this.x = newX;
      isUpdated = true;
    }

    if (newY !== this.y) {
      this.y = newY;
      isUpdated = true;
    }

    if (isUpdated && this.onUpdateCallback) {
      this.onUpdateCallback(this.x, this.y);
    }
  }
}
