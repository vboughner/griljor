// Image cache: URL -> processed ImageData (white=transparent applied)
const imageCache = new Map<string, ImageData>();
const loadingPromises = new Map<string, Promise<ImageData | null>>();

/**
 * Load a PNG, convert white pixels to transparent, return ImageData.
 * Returns null if the image fails to load (missing bitmap).
 */
function loadAndProcess(url: string): Promise<ImageData | null> {
  if (loadingPromises.has(url)) return loadingPromises.get(url)!;

  const p = new Promise<ImageData | null>((resolve) => {
    const img = new Image();
    img.onload = () => {
      const oc = new OffscreenCanvas(img.width, img.height);
      const ctx = oc.getContext('2d')!;
      ctx.drawImage(img, 0, 0);
      const id = ctx.getImageData(0, 0, img.width, img.height);
      // White (255,255,255) → transparent; grayscale bitmaps
      for (let i = 0; i < id.data.length; i += 4) {
        const r = id.data[i];
        // Treat near-white as transparent (threshold 200)
        if (r >= 200 && id.data[i + 1] >= 200 && id.data[i + 2] >= 200) {
          id.data[i + 3] = 0;
        } else {
          // Make dark pixels opaque with slight tint for visibility
          id.data[i + 3] = 255;
        }
      }
      imageCache.set(url, id);
      resolve(id);
    };
    img.onerror = () => resolve(null);
    img.src = url;
  });

  loadingPromises.set(url, p);
  return p;
}

/**
 * Load a bitmap and apply its mask. The mask PNG: black pixels = opaque area,
 * white pixels = transparent. Returns processed ImageData.
 */
export async function loadMaskedSprite(
  bitmapUrl: string,
  maskUrl: string
): Promise<ImageData | null> {
  const cacheKey = `masked:${bitmapUrl}`;
  if (imageCache.has(cacheKey)) return imageCache.get(cacheKey)!;
  if (loadingPromises.has(cacheKey)) return loadingPromises.get(cacheKey)!;

  const p = Promise.all([loadAndProcess(bitmapUrl), loadAndProcess(maskUrl)]).then(
    ([bitmapData, maskData]) => {
      if (!bitmapData) return null;

      const result = new ImageData(
        new Uint8ClampedArray(bitmapData.data),
        bitmapData.width,
        bitmapData.height
      );

      if (maskData) {
        // Zero alpha where mask pixel is white (not part of sprite)
        for (let i = 0; i < result.data.length; i += 4) {
          if (maskData.data[i] >= 200) {
            result.data[i + 3] = 0;
          }
        }
      }

      imageCache.set(cacheKey, result);
      return result;
    }
  );

  loadingPromises.set(cacheKey, p);
  return p;
}

/**
 * Load a plain (unmasked) bitmap with white→transparent conversion.
 */
export async function loadSprite(url: string): Promise<ImageData | null> {
  return loadAndProcess(url);
}

/**
 * Preload a set of URLs. Resolves when all are done (ignores failures).
 */
export async function preloadAll(urls: string[]): Promise<void> {
  await Promise.all(urls.map((u) => loadAndProcess(u)));
}

export function getCached(url: string): ImageData | undefined {
  return imageCache.get(url) ?? imageCache.get(`masked:${url}`);
}
