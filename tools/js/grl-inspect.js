//
// grl-inspect.js
// JavaScript's clone of grl-inspect.c
//
// Copyright (C) 2010, Igalia S.L.
//
// Author: Simón Pena <spenap@gmail.com>
//

const Grl = imports.gi.Grl;
const MainLoop = imports.mainloop;

const FLICKR_KEY = "fa037bee8120a921b34f8209d715a2fa";
const FLICKR_SECRET = "9f6523b9c52e3317";
const VIMEO_KEY = "4d908c69e05a9d5b5c6669d302f920cb";
const VIMEO_SECRET = "4a923ffaab6238eb";
const YOUTUBE_KEY = "AI39si4EfscPllSfUy1IwexMf__kntTL_G5dfSr2iUEVN45RHGq92Aq0lX25OlnOkG6KTN-4soVAkAf67fWYXuHfVADZYr7S1A";

function list_all_sources () {

    let registry = Grl.PluginRegistry.get_default ();
    let sources = registry.get_sources (false);

    for each (let source in sources) {
        log (source.get_id () + ": " + "type casting missing");
    }
}

function print_keys (keys) {

    let printed_keys = ''
    for each (let key in keys) {
        printed_keys += Grl.metadata_key_get_name (key) + ", ";
    }
    log (printed_keys);
}

function introspect_source (source_id) {

    let registry = Grl.PluginRegistry.get_default ();
    let source = registry.lookup_source (source_id);

    if (source != null) {

        log ("Plugin Details:");
        log ("  Identifier:" + source.get_id ());
        log ("    Filename:" + source.get_filename ());
        log ("        Rank:" + source.get_rank ());

        let info_keys = source.get_info_keys ();

        for each (let info_key in info_keys) {
            log ("  " + info_key + " " + source.get_info (info_key));
        }
        log ("");

        log ("Source Details:");
        log ("    Identifier: " + source.get_id ());
        log ("          Type: " + "check type??");
        log ("          Name: " + source.get_name ());
        log ("   Description: " + source.get_description ());
        log ("");

        let supported_ops = source.supported_operations ();

        log ("Supported operations:");
        if (supported_ops & Grl.SupportedOps.RESOLVE) {
            log ("  grl_metadata_source_resolve():\tResolve Metadata\n");
        }
        if (supported_ops & Grl.SupportedOps.METADATA) {
            log ("  grl_media_source_metadata():\t\tRetrieve Metadata\n");
        }
        if (supported_ops & Grl.SupportedOps.BROWSE) {
            log ("  grl_media_source_browse():\t\tBrowse\n");
        }
        if (supported_ops & Grl.SupportedOps.SEARCH) {
            log ("  grl_media_source_search():\t\tSearch\n");
        }
        if (supported_ops & Grl.SupportedOps.QUERY) {
            log ("  grl_media_source_query():\t\tQuery\n");
        }
        if (supported_ops & Grl.SupportedOps.STORE) {
            log ("  grl_metadata_source_set_metadata():\tUpdate Metadata\n");
        }
        if (supported_ops & Grl.SupportedOps.STORE_PARENT) {
            log ("  grl_media_source_store():\t\tAdd New Media\n");
        }
        if (supported_ops & Grl.SupportedOps.REMOVE) {
            log ("  grl_media_source_remove():\t\tRemove Media\n");
        }

        log ("Supported keys:");
        log ("  Readable Keys:\t\t");
        print_keys (source.supported_keys ());
        log ("  Writable Keys:\t\t");
        print_keys (source.writable_keys ());
    }
}

function init () {

    let registry = Grl.PluginRegistry.get_default ();

    configure_flickr ();
    configure_vimeo ();
    configure_youtube ();

    registry.load_all ();
}

function configure_flickr () {

    let registry = Grl.PluginRegistry.get_default ();
    let flickr_config = Grl.Config.new ('grl-flickr', null);
    flickr_config.set_api_key (FLICKR_KEY);
    flickr_config.set_api_secret (FLICKR_SECRET);
    registry.add_config (flickr_config);
}

function configure_vimeo () {

    let registry = Grl.PluginRegistry.get_default ();
    let vimeo_config = Grl.Config.new ('grl-vimeo', null);
    vimeo_config.set_api_key (VIMEO_KEY);
    vimeo_config.set_api_secret (VIMEO_SECRET);
    registry.add_config (vimeo_config);
}

function configure_youtube () {

    let registry = Grl.PluginRegistry.get_default ();
    let youtube_config = Grl.Config.new ('grl-youtube', null);
    youtube_config.set_api_key (YOUTUBE_KEY);
    registry.add_config (youtube_config);
}

function run () {

    init ();

    if (ARGV[0] == null) {
        list_all_sources ();
    } else {
        introspect_source (ARGV[0]);
    }

    MainLoop.quit ("main");
}

Grl.init (null, null);
MainLoop.idle_add(run);

MainLoop.run ("main");
