export default class Api {
  /**
   * Api constructor, initializes backend url.
   * @constructor
   */
  constructor() {
    this.api_url_base = new URL('https://api.edamam.com/search');
  }

  get = async (from, to) => {
    const url = new URL(this.api_url_base);
    
    const params = {
      // method: "GET",
      app_id: "900da95e",
      app_key: "40698503668e0bb3897581f4766d77f9",
      q: "pizza",
      from: from,
      to: to
    }

    url.search = new URLSearchParams(params)
    const res = await fetch(url);
    return await res.json();
  }
}