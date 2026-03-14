export type ColorMode = 'dark' | 'light';

let colorMode: ColorMode = 'dark';

export function setColorMode(mode: ColorMode): void {
  colorMode = mode;
}

export function getColorMode(): ColorMode {
  return colorMode;
}

const imageCache = new Map<string, ImageData>();
const loadingPromises = new Map<string, Promise<ImageData | null>>();

/** Load a PNG and return raw ImageData with no color transforms. */
function loadRaw(url: string): Promise<ImageData | null> {
  const key = `raw:${url}`;
  if (loadingPromises.has(key)) return loadingPromises.get(key)!;
  const p = new Promise<ImageData | null>((resolve) => {
    const img = new Image();
    img.onload = () => {
      const oc = new OffscreenCanvas(img.width, img.height);
      const ctx = oc.getContext('2d')!;
      ctx.drawImage(img, 0, 0);
      resolve(ctx.getImageData(0, 0, img.width, img.height));
    };
    img.onerror = () => resolve(null);
    img.src = url;
  });
  loadingPromises.set(key, p);
  return p;
}

/**
 * Load a bitmap PNG and apply the current color mode transform:
 *   dark:  white → transparent, dark pixels inverted to bright
 *   light: white → transparent, dark pixels kept as-is (black on white bg)
 * Cache is keyed by mode so both can coexist.
 */
function loadAndProcess(url: string, mode: ColorMode): Promise<ImageData | null> {
  const key = `${mode}:${url}`;
  if (loadingPromises.has(key)) return loadingPromises.get(key)!;

  const p = new Promise<ImageData | null>((resolve) => {
    const img = new Image();
    img.onload = () => {
      const oc = new OffscreenCanvas(img.width, img.height);
      const ctx = oc.getContext('2d')!;
      ctx.drawImage(img, 0, 0);
      const id = ctx.getImageData(0, 0, img.width, img.height);
      for (let i = 0; i < id.data.length; i += 4) {
        const r = id.data[i];
        if (r >= 200 && id.data[i + 1] >= 200 && id.data[i + 2] >= 200) {
          id.data[i + 3] = 0; // white → transparent in both modes
        } else if (mode === 'dark') {
          const v = 255 - r; // invert: black → white on dark bg
          id.data[i] = v;
          id.data[i + 1] = v;
          id.data[i + 2] = v;
          id.data[i + 3] = 255;
        } else {
          id.data[i + 3] = 255; // light: keep original dark pixels as-is
        }
      }
      imageCache.set(key, id);
      resolve(id);
    };
    img.onerror = () => resolve(null);
    img.src = url;
  });

  loadingPromises.set(key, p);
  return p;
}

export async function loadMaskedSprite(
  bitmapUrl: string,
  maskUrl: string,
): Promise<ImageData | null> {
  const mode = colorMode;
  const cacheKey = `${mode}:masked:${bitmapUrl}`;
  if (imageCache.has(cacheKey)) return imageCache.get(cacheKey)!;
  if (loadingPromises.has(cacheKey)) return loadingPromises.get(cacheKey)!;

  const p = Promise.all([loadRaw(bitmapUrl), loadRaw(maskUrl)]).then(([bitmapData, maskData]) => {
    if (!bitmapData) return null;

    const result = new ImageData(
      new Uint8ClampedArray(bitmapData.data),
      bitmapData.width,
      bitmapData.height,
    );

    for (let i = 0; i < result.data.length; i += 4) {
      const outsideMask = !maskData || maskData.data[i] >= 200;
      if (outsideMask) {
        // Outside sprite silhouette → transparent
        result.data[i + 3] = 0;
      } else {
        // Inside sprite silhouette: features (dark bits) vs interior (light bits)
        const isFeature = result.data[i] < 200;
        const v =
          mode === 'dark'
            ? isFeature
              ? 255
              : 0 // dark mode: white features, black interior
            : isFeature
              ? 0
              : 255; // light mode: black features, white interior
        result.data[i] = v;
        result.data[i + 1] = v;
        result.data[i + 2] = v;
        result.data[i + 3] = 255;
      }
    }

    imageCache.set(cacheKey, result);
    return result;
  });

  loadingPromises.set(cacheKey, p);
  return p;
}

export async function loadSprite(url: string): Promise<ImageData | null> {
  return loadAndProcess(url, colorMode);
}

/**
 * Load a bitmap tile for room rendering (floor/wall layers) with no mask.
 * Unlike loadSprite, white pixels are rendered as the map background color
 * (opaque), matching the original X11 XCopyPlane behavior where both bit
 * values always drew an opaque pixel.
 */
export async function loadOpaqueTile(url: string): Promise<ImageData | null> {
  const mode = colorMode;
  const key = `${mode}:opaque:${url}`;
  if (imageCache.has(key)) return imageCache.get(key)!;
  if (loadingPromises.has(key)) return loadingPromises.get(key)!;

  const bgR = mode === 'dark' ? 51 : 232; // #333 vs #e8e8e8
  const p = loadRaw(url).then((raw) => {
    if (!raw) return null;
    const result = new ImageData(new Uint8ClampedArray(raw.data), raw.width, raw.height);
    for (let i = 0; i < result.data.length; i += 4) {
      const isLight = raw.data[i] >= 200 && raw.data[i + 1] >= 200 && raw.data[i + 2] >= 200;
      if (isLight) {
        result.data[i] = bgR;
        result.data[i + 1] = bgR;
        result.data[i + 2] = bgR;
        result.data[i + 3] = 255;
      } else {
        const v = mode === 'dark' ? 255 - raw.data[i] : raw.data[i];
        result.data[i] = v;
        result.data[i + 1] = v;
        result.data[i + 2] = v;
        result.data[i + 3] = 255;
      }
    }
    imageCache.set(key, result);
    return result;
  });

  loadingPromises.set(key, p);
  return p;
}
